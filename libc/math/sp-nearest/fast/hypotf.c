/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * hypotf.c
 *
 * Created on: Nov 16, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * sqrt(a^2 + b^2) for a and b in [2^-20, 2^62), in either order, so that both squares and their errors are normal: the
 * squares as exact pairs (a fused multiply-add gives each error), their sum h + l (a two-sum of the high parts, which
 * needs no ordering, the low parts added), s = sqrtf(h), and in *d the residual h - s^2 (exact by an fma) plus l, so that
 * the root is s + *d/(2 s) to about 2^-46 (PLAN D219; docs/research/libm-fast-double.md, section 30)
 */
[[gnu::always_inline]] static inline float hypotf_root(float a, float b, float *d)
{
	float ah = a * a;
	float al = __builtin_fmaf(a, a, -ah);
	float bh = b * b;
	float bl = __builtin_fmaf(b, b, -bh);
	float h = ah + bh;
	float t = h - ah;
	float l = ((ah - (h - t)) + (bh - t)) + (al + bl);
	float s = __builtin_sqrtf(h);

	*d = __builtin_fmaf(-s, s, h) + l;

	return s;
}

/* m 2^shift as a 128-bit integer, hi:lo, for m below 2^64 and 0 <= shift < 64 */
static inline void hypotf_shifted(uint64_t m, int shift, uint64_t *hi, uint64_t *lo)
{
	*lo = m << shift;
	*hi = shift == 0 ? 0 : m >> (64 - shift);
}

/*
 * The rounding of s + c, c within about 2^-17 of half an ulp of s (sign of c): a^2 + b^2 against the square of the midpoint
 * s +- u/2, exactly in integers: a = ma 2^ea, b = mb 2^eb, s = ms 2^es (24-bit significands), the midpoint (2 ms +- 1)
 * 2^(es - 1), all squares at the base 4^e0, e0 the lowest of ea, eb and es - 1 (a result this close to a midpoint has the
 * smaller operand within 2^12 of the larger, so every shift is below 64). Ties to even.
 */
[[gnu::noinline]] static float hypotf_tie(float a, float b, float s, float c)
{
	uint32_t ia = __libc_float_bits(a);
	uint32_t ib = __libc_float_bits(b);
	uint32_t is = __libc_float_bits(s);
	uint64_t ma = (ia & 0x7fffffu) | 0x800000u;
	uint64_t mb = (ib & 0x7fffffu) | 0x800000u;
	uint64_t mm = 2 * (uint64_t)((is & 0x7fffffu) | 0x800000u) + (c > 0 ? 1 : (uint64_t)-1);
	int ea = (int)(ia >> 23);
	int eb = (int)(ib >> 23);
	int es = (int)(is >> 23);
	int e0 = (ea < eb ? ea : eb) < es - 1 ? (ea < eb ? ea : eb) : es - 1;
	uint64_t bh;
	uint64_t bl;
	uint64_t xh;
	uint64_t xl;
	uint64_t yh;
	uint64_t yl;
	float away = c > 0 ? __libc_float_from(is + 1) : __libc_float_from(is - 1);

	/* H = ma^2 4^(ea - e0) + mb^2 4^(eb - e0); the midpoint's square (mm 2^-1)^2 4^(es - e0) = mm^2 4^(es - 1 - e0) */
	hypotf_shifted(ma * ma, 2 * (ea - e0), &xh, &xl);
	hypotf_shifted(mb * mb, 2 * (eb - e0), &bh, &bl);
	xl += bl;
	xh += bh + (xl < bl);
	hypotf_shifted(mm * mm, 2 * (es - 1 - e0), &yh, &yl);
	if (xh != yh || xl != yl)
		return (xh > yh || (xh == yh && xl > yl)) == (c > 0) ? away : s;

	return (is & 1) == 0 ? s : away;
}

/*
 * The root rounded once, correctly: s + c with c = d/(2 s) (a quotient, so that an exact root raises no inexact), within
 * about 2^-46 of the root, so only a result within about 2^-17 of a midpoint (an exact one is a Pythagorean tie) is
 * decided exactly; *c the correction, for a caller that scales the result into the subnormals
 */
[[gnu::always_inline]] static inline float hypotf_rounded(float a, float b, float *s, float *c)
{
	float d;
	uint32_t half;

	*s = hypotf_root(a, b, &d);
	*c = d / (*s + *s);
	/* |c| within 64 units of the bits of u/2, a power of two: within 2^-17 of it either side (2^-18 below) */
	half = (__libc_float_bits(*s) & 0x7f800000u) - (24u << 23);
	if ((__libc_float_bits(*c) & 0x7fffffffu) - half + 64 <= 128)
		return hypotf_tie(a, b, *s, *c);

	/* c's own rounding moves the sum only within 2^-23 ulp of a midpoint, which the test above takes */
	return *s + *c;
}

/*
 * The arguments off the common path, ordered by magnitude: an infinity wins even over a NaN (C23 F.10.4.4), then a NaN, then b below 2^-12 a
 * (b^2/(2 a^2) is below a quarter of an ulp of a: the result rounds to a, b 2^-24 added only to raise inexact), then a or b scaled by powers
 * of two into the common path's range (exact), the result scaled back with one rounding, subnormal results included
 */
[[gnu::noinline]] static float hypotf_wide(uint32_t ia, uint32_t ib)
{
	float a;
	float b;
	float r;
	float s;
	float c;
	int scale = 0;

	if (ia < ib) {
		uint32_t swap = ia;

		ia = ib;
		ib = swap;
	}
	if (ia == 0x7f800000u || ib == 0x7f800000u)
		return INFINITY;
	a = __libc_float_from(ia);
	b = __libc_float_from(ib);
	if (ia > 0x7f800000u)
		return a + b;
	/* A zero b gives a exactly (+0 for two zeros, whose scaling below would never end) */
	if (ib == 0)
		return a;
	if (ia - ib >= (14u << 23))
		return a + b * 0x1p-24f;
	if (ia >= 0x5e800000u) {
		a *= 0x1p-70f;
		b *= 0x1p-70f;
		scale = 70;
	}
	/* At most three steps from the smallest subnormal */
	while (__libc_float_bits(a) < 0x35800000u) {
		a *= 0x1p64f;
		b *= 0x1p64f;
		scale -= 64;
	}
	r = hypotf_rounded(a, b, &s, &c);
	/* A normal result scales back exactly; a subnormal one takes the unrounded pair, rounded once (no exact tie there) */
	if ((int)(__libc_float_bits(r) >> 23) + scale >= 1)
		return __libc_exp2f_scale((struct __libc_ff){ .hi = r, .lo = 0.0f }, scale);

	return __libc_exp2f_scale(__libc_ff_fast_sum(s, c), scale);
}

/*
 * The fast hypotf of the single-precision FPU cores (PLAN D162, D219), correctly rounded: one test sends everything but
 * the common range to hypotf_wide, then hypotf_rounded (the note, section 30). The strict file is
 * within 0.509 ulp (D59). C23 7.12.7.4, F.10.4.4.
 */
float hypotf(float x, float y)
{
	uint32_t ia = __libc_float_bits(x) & 0x7fffffffu;
	uint32_t ib = __libc_float_bits(y) & 0x7fffffffu;
	float s;
	float c;

	/* Both in [2^-20, 2^62): no ordering and no gap test needed (hypotf_root) */
	if (ia - 0x35800000u >= 0x5e800000u - 0x35800000u || ib - 0x35800000u >= 0x5e800000u - 0x35800000u)
		return hypotf_wide(ia, ib);

	return hypotf_rounded(__libc_float_from(ia), __libc_float_from(ib), &s, &c);
}
