/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * hypot.c
 *
 * Created on: Nov 16, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* The squares' pieces and the root: H = h + mid + al + bl exactly, s = sqrt(h), the correction c = d/(2 s) */
struct hypot_root {
	double h;
	double mid;
	double al;
	double bl;
	double s;
	double c;
	double d;
};

/*
 * sqrt(a^2 + b^2) for a >= b > 0, a in [2^-400, 2^500) and b within 2^27 of a, so that every piece below is normal: the
 * squares as exact pairs (__libc_fast_fused_product: an fma inline, or Dekker's product), h + mid their high parts' exact sum (a fast
 * two-sum), s = sqrt(h), d = h - s^2 (exact: s^2 is within an ulp of h) plus the low parts, so that the root is
 * s + c, c = d/(2 s), to about 2^-100 (a quotient rather than d times 1/(2 s), so that an exact root raises no inexact)
 */
[[gnu::always_inline]] static inline void hypot_root(double a, double b, struct hypot_root *r)
{
	struct __libc_dd a2 = __libc_fast_fused_product(a, a);
	struct __libc_dd b2 = __libc_fast_fused_product(b, b);
	struct __libc_dd s2;

	r->h = a2.hi + b2.hi;
	r->mid = (a2.hi - r->h) + b2.hi;
	r->al = a2.lo;
	r->bl = b2.lo;
	r->s = __builtin_sqrt(r->h);
	/* h - s^2 exactly: one fused multiply-add (s^2 is within an ulp of h), or the square as Dekker's pair */
	if (__libc_fpu_fma) {
		r->d = __builtin_fma(-r->s, r->s, r->h) + (r->mid + (r->al + r->bl));
	} else {
		s2 = __libc_dd_product(r->s, r->s);
		r->d = ((r->h - s2.hi) - s2.lo) + (r->mid + (r->al + r->bl));
	}
	r->c = r->d / (r->s + r->s);
}

/* e (an expansion of n nonoverlapping parts, smallest first) plus t, exactly (Shewchuk's grow-expansion); returns n + 1 */
static inline int hypot_grow(double *e, int n, double t)
{
	for (int i = 0; i < n; ++i) {
		struct __libc_dd sum = __libc_dd_sum(t, e[i]);

		e[i] = sum.lo;
		t = sum.hi;
	}
	e[n] = t;

	return n + 1;
}

/*
 * The rounding of s + c, c within 2^-40 ulp of half an ulp u of s with c's sign: the sign of H - m^2 for the midpoint
 * m = s +- u/2, exactly, as the seven-part expansion (h - s^2's high part) - s^2's low part + mid + al + bl -+ s u - u^2/4
 * (each part exact: s u and u^2/4 scale by powers of two). Ties to even.
 */
[[gnu::noinline]] static double hypot_tie(const struct hypot_root *r)
{
	struct __libc_dd s2 = __libc_fast_fused_product(r->s, r->s);
	double u = __libc_double_from((__libc_double_bits(r->s) & 0x7ff0000000000000ull) - (52ull << 52));
	double sign = r->c > 0 ? 1.0 : -1.0;
	double away = r->s + sign * u;
	double e[7];
	int n = 0;

	n = hypot_grow(e, n, r->h - s2.hi);
	n = hypot_grow(e, n, -s2.lo);
	n = hypot_grow(e, n, r->mid);
	n = hypot_grow(e, n, r->al);
	n = hypot_grow(e, n, r->bl);
	n = hypot_grow(e, n, -sign * r->s * u);
	n = hypot_grow(e, n, -0.25 * u * u);
	while (n > 0 && e[n - 1] == 0)
		--n;
	if (n > 0)
		return (e[n - 1] > 0) == (sign > 0) ? away : r->s;

	return (__libc_double_bits(r->s) & 1) == 0 ? r->s : away;
}

/* The tie test for the common path, out of line and from a and b, so that the common path keeps its pieces in registers */
[[gnu::noinline]] static double hypot_tie_from(double a, double b)
{
	struct hypot_root r;

	hypot_root(a, b, &r);

	return hypot_tie(&r);
}

/* The root rounded once, correctly: s + c, or the tie test within 2^-40 ulp of a midpoint (an exact one is a tie) */
[[gnu::always_inline]] static inline double hypot_rounded(double a, double b, struct hypot_root *r)
{
	double u;

	hypot_root(a, b, r);
	u = __libc_double_from((__libc_double_bits(r->s) & 0x7ff0000000000000ull) - (52ull << 52));
	if (__builtin_fabs(__builtin_fabs(r->c) - 0.5 * u) <= 0x1p-40 * u)
		return hypot_tie(r);

	/* c's own rounding moves the sum only within 2^-52 ulp of a midpoint, which the test above takes */
	return r->s + r->c;
}

/*
 * hypot_rounded for the common path (P11.08d4, D249): the same root, with the midpoint test on the high words alone,
 * which takes |c| within 2^-20 of half an ulp of s (high word that of u/2 or the one below), a superset of
 * hypot_rounded's 2^-40 window, about once in 2^19 calls, and the tie test taking a and b
 */
[[gnu::always_inline]] static inline double hypot_common(double a, double b)
{
	struct hypot_root r;
	uint32_t half;
	uint32_t high;

	hypot_root(a, b, &r);
	half = ((uint32_t)(__libc_double_bits(r.s) >> 32) & 0x7ff00000u) - (53u << 20);
	high = (uint32_t)(__libc_double_bits(r.c) >> 32) & 0x7fffffffu;
	if (high - (half - 1u) <= 1u)
		return hypot_tie_from(a, b);

	return r.s + r.c;
}

/*
 * The arguments off the common path: an infinity wins even over a NaN (C23 F.10.4.4), then a NaN; a zero b gives a; b
 * below 2^-27 a (b^2/(2 a^2) is below a quarter of an ulp of a) rounds to a, b 2^-60 added only to raise inexact. Then a
 * and b scaled by powers of two into the common range (exact): a normal result scales back exactly; a subnormal one is
 * rounded once onto the subnormal grid g, from s rounded to it (s + 2^52 g keeps the ulp g, s being below 2^52 g) and the
 * remainder (s - that) + c (no exact tie there: H
 * is a multiple of 2^-2148, a midpoint's square is not).
 */
[[gnu::noinline]] static double hypot_wide(uint64_t ia, uint64_t ib)
{
	struct hypot_root r;
	double a;
	double b;
	double v;
	double g;
	double grid;
	double rest;
	int scale = 0;

	if (ia == 0x7ff0000000000000ull || ib == 0x7ff0000000000000ull)
		return INFINITY;
	a = __libc_double_from(ia);
	b = __libc_double_from(ib);
	if (ia > 0x7ff0000000000000ull)
		return a + b;
	if (ib == 0)
		return a;
	if (ia - ib >= (28ull << 52))
		return a + b * 0x1p-60;
	if (ia >= 0x5f30000000000000ull) {
		a *= 0x1p-600;
		b *= 0x1p-600;
		scale = 600;
	}
	/* At most two steps from the smallest subnormal */
	while (__libc_double_bits(a) < 0x26f0000000000000ull) {
		a *= 0x1p600;
		b *= 0x1p600;
		scale -= 600;
	}
	v = hypot_rounded(a, b, &r);
	if (scale > 0)
		return v * 0x1p600;
	if (scale == 0)
		return v;
	/* The result's grid at the scaled size: 2^-1074 2^-scale; a normal result scales back exactly */
	g = __libc_double_from((uint64_t)(-1074 - scale + 1023) << 52);
	if (v >= 0x1p52 * g)
		return scale == -600 ? v * 0x1p-600 : v * 0x1p-600 * 0x1p-600;
	grid = (r.s + 0x1p52 * g) - 0x1p52 * g;
	rest = (r.s - grid) + r.c;
	if (rest > 0.5 * g)
		grid += g;
	else if (rest < -0.5 * g)
		grid -= g;
	/* The scaling back is exact, so a rounded subnormal result raises underflow and inexact itself (F.10.4.4) */
	if (rest != 0)
		__libc_math_raise_underflow();

	return scale == -600 ? grid * 0x1p-600 : grid * 0x1p-600 * 0x1p-600;
}

/*
 * The fast hypot of the double-FPU cores (PLAN D162, D219), correctly rounded: ordered by magnitude, one test sends
 * everything but the common range to hypot_wide, then hypot_rounded (docs/research/libm-fast-double.md, section 30).
 * C23 7.12.7.4, F.10.4.4.
 */
double hypot(double x, double y)
{
	double fx = __builtin_fabs(x);
	double fy = __builtin_fabs(y);
	/* Ordered by a quiet comparison (P11.08d4, D249): with a NaN either way, the tests below send it to hypot_wide */
	bool swap = isless(fx, fy);
	double a = swap ? fy : fx;
	double b = swap ? fx : fy;
	uint32_t ha = (uint32_t)(__libc_double_bits(a) >> 32);
	uint32_t hb = (uint32_t)(__libc_double_bits(b) >> 32);

	/* a in [2^-400, 2^500) and b within 2^27 of it, on the high words */
	if (ha - 0x26f00000u >= 0x5f300000u - 0x26f00000u || ha - hb >= (28u << 20))
		return hypot_wide(__libc_double_bits(a), __libc_double_bits(b));

	return hypot_common(a, b);
}
