/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pow.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* A 128-bit unsigned value, two words */
struct pow_wide {
	uint64_t hi;
	uint64_t lo;
};

/* A positive value m 2^(e - 95), m of 96 bits with its top bit set (the top 64 in hi, the rest in lo) */
struct pow_float {
	uint64_t hi;
	uint32_t lo;
	int32_t e;
};

/* log2(e) as a pow_float, rounded */
static const struct pow_float pow_log2e = { 0xb8aa3b295c17f0bbull, 0xbe87fed0u, 0 };

/* a b exactly */
static inline struct pow_wide pow_wide_product(uint64_t a, uint64_t b)
{
	uint64_t p00 = __libc_trig_wide((uint32_t)a, (uint32_t)b);
	uint64_t p01 = __libc_trig_wide((uint32_t)a, (uint32_t)(b >> 32));
	uint64_t p10 = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)b);
	uint64_t middle = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;

	return (struct pow_wide){ .hi = __libc_trig_wide((uint32_t)(a >> 32), (uint32_t)(b >> 32)) + (p01 >> 32) + (p10 >> 32) + (middle >> 32), .lo = (middle << 32) | (uint32_t)p00 };
}

static inline struct pow_wide pow_wide_add(struct pow_wide a, struct pow_wide b)
{
	uint64_t lo = a.lo + b.lo;

	return (struct pow_wide){ .hi = a.hi + b.hi + (lo < a.lo), .lo = lo };
}

static inline struct pow_wide pow_wide_negate(struct pow_wide a)
{
	return (struct pow_wide){ .hi = ~a.hi + (a.lo == 0), .lo = -a.lo };
}

/* The top 96 bits of a b, normalized: the 192-bit product's columns from 2^64 up (the lower ones change it by 2^-125) */
static inline struct pow_float pow_float_product(struct pow_float a, struct pow_float b)
{
	uint32_t a2 = (uint32_t)(a.hi >> 32);
	uint32_t a1 = (uint32_t)a.hi;
	uint32_t b2 = (uint32_t)(b.hi >> 32);
	uint32_t b1 = (uint32_t)b.hi;
	uint64_t p20 = __libc_trig_wide(a2, b.lo);
	uint64_t p11 = __libc_trig_wide(a1, b1);
	uint64_t p02 = __libc_trig_wide(a.lo, b2);
	uint64_t p21 = __libc_trig_wide(a2, b1);
	uint64_t p12 = __libc_trig_wide(a1, b2);
	uint64_t p22 = __libc_trig_wide(a2, b2);
	uint64_t c2 = (uint64_t)(uint32_t)p20 + (uint32_t)p11 + (uint32_t)p02;
	uint64_t c3 = (p20 >> 32) + (p11 >> 32) + (p02 >> 32) + (uint32_t)p21 + (uint32_t)p12 + (c2 >> 32);
	uint64_t c4 = (p21 >> 32) + (p12 >> 32) + (uint32_t)p22 + (c3 >> 32);
	uint64_t hi = (((p22 >> 32) + (c4 >> 32)) << 32) | (uint32_t)c4;
	uint64_t lo = ((uint64_t)(uint32_t)c3 << 32) | (uint32_t)c2;

	if ((hi >> 63) != 0)
		return (struct pow_float){ .hi = hi, .lo = (uint32_t)(lo >> 32), .e = a.e + b.e + 1 };

	return (struct pow_float){ .hi = (hi << 1) | (lo >> 63), .lo = (uint32_t)(lo >> 31), .e = a.e + b.e };
}

/* v >> shift, shift >= 0 */
static inline struct pow_wide pow_wide_shift(struct pow_wide v, int32_t shift)
{
	if (shift >= 128)
		return (struct pow_wide){ .hi = 0, .lo = 0 };
	if (shift >= 64)
		return (struct pow_wide){ .hi = 0, .lo = v.hi >> (shift - 64) };
	if (shift == 0)
		return v;

	return (struct pow_wide){ .hi = v.hi >> shift, .lo = (v.lo >> shift) | (v.hi << (64 - shift)) };
}

/* A pow_float's 96-bit m as a 128-bit value */
static inline struct pow_wide pow_float_significand(struct pow_float a)
{
	return (struct pow_wide){ .hi = a.hi >> 32, .lo = (a.hi << 32) | a.lo };
}

/* The nonzero v 2^-point as a pow_float */
static inline struct pow_float pow_float_from(struct pow_wide v, int32_t point)
{
	int32_t top = v.hi != 0 ? 127 - __builtin_clzll(v.hi) : 63 - __builtin_clzll(v.lo);
	int32_t shift = 127 - top;
	uint64_t hi;
	uint64_t lo;

	/* v << shift, its top bit then at 127 */
	if (shift >= 64) {
		hi = v.lo << (shift - 64);
		lo = 0;
	} else if (shift > 0) {
		hi = (v.hi << shift) | (v.lo >> (64 - shift));
		lo = v.lo << shift;
	} else {
		hi = v.hi;
		lo = v.lo;
	}

	return (struct pow_float){ .hi = hi, .lo = (uint32_t)(lo >> 32), .e = top - point };
}

/*
 * ln z in Q126 for z in [45/64, 90/64) in Q63, to about 2^-82 absolutely: -log(invc) from the table to Q96 (its tail),
 * r = z invc - 1 exact (the 96-bit product), r^2/2 from r to 2^-71, and r^3 S(r) with S = sum of (-r)^m/(m + 3), m <= 8,
 * by Horner's rule in y = 2^7 |r| in Q64 (unsigned, subtracting for r > 0)
 */
static inline struct pow_wide pow_log_table(uint64_t z)
{
	const struct __libc_dp_fixed_log_entry *entry = &__libc_dp_fixed_log_table[(z - 0x5a00000000000000ull) >> 56];
	uint64_t low = __libc_trig_wide((uint32_t)z, entry->invc);
	uint64_t high = __libc_trig_wide((uint32_t)(z >> 32), entry->invc) + (low >> 32);
	/* r 2^62 by its floor, and the next 32 bits of r 2^94 */
	int64_t rh = (int64_t)(high - 0x4000000000000000ull);
	struct pow_wide r = { .hi = (uint64_t)rh, .lo = (uint64_t)(uint32_t)low << 32 };
	struct pow_wide magnitude = rh < 0 ? pow_wide_negate(r) : r;
	uint64_t y = (magnitude.hi << 9) | (magnitude.lo >> 55);
	struct pow_wide square = pow_wide_product(y, y);
	struct pow_wide sum;
	uint64_t y3;
	uint64_t s;

	if (rh < 0) {
		s = 0x0000000000000ccdull + __libc_trig_high64(y, 0x0000000000000017ull);
		s = 0x0000000000071c72ull + __libc_trig_high64(y, s);
		s = 0x0000000004000000ull + __libc_trig_high64(y, s);
		s = 0x0000000249249249ull + __libc_trig_high64(y, s);
		s = 0x0000015555555555ull + __libc_trig_high64(y, s);
		s = 0x0000cccccccccccdull + __libc_trig_high64(y, s);
		s = 0x0080000000000000ull + __libc_trig_high64(y, s);
		s = 0x5555555555555555ull + __libc_trig_high64(y, s);
	} else {
		s = 0x0000000000000ccdull - __libc_trig_high64(y, 0x0000000000000017ull);
		s = 0x0000000000071c72ull - __libc_trig_high64(y, s);
		s = 0x0000000004000000ull - __libc_trig_high64(y, s);
		s = 0x0000000249249249ull - __libc_trig_high64(y, s);
		s = 0x0000015555555555ull - __libc_trig_high64(y, s);
		s = 0x0000cccccccccccdull - __libc_trig_high64(y, s);
		s = 0x0080000000000000ull - __libc_trig_high64(y, s);
		s = 0x5555555555555555ull - __libc_trig_high64(y, s);
	}
	/* |r|^3 S = y^3 S 2^-21: in Q126, (y^3 S in Q64) 2^41 */
	y3 = __libc_trig_high64(square.hi, y);
	s = __libc_trig_high64(y3, s);

	/* -log(invc) to Q96, as Q126 */
	sum.hi = (uint64_t)(entry->logc >> 2);
	sum.lo = ((uint64_t)entry->logc << 62) + ((uint64_t)(int64_t)entry->tail << 30);
	sum.hi += (uint64_t)((int64_t)entry->tail >> 34) + (sum.lo < ((uint64_t)entry->logc << 62));
	sum = pow_wide_add(sum, r);
	/* - r^2/2: y^2 = r^2 2^142, so r^2/2 in Q126 is y^2 2^-17 */
	sum = pow_wide_add(sum, pow_wide_negate((struct pow_wide){ .hi = square.hi >> 17, .lo = (square.hi << 47) | (square.lo >> 17) }));
	/* + r^3 S, the sign of r */
	{
		struct pow_wide d = { .hi = s >> 23, .lo = s << 41 };

		sum = pow_wide_add(sum, rh < 0 ? pow_wide_negate(d) : d);
	}

	return sum;
}

/*
 * log1p(s) as a pow_float for s = t 2^-63 strictly within 2^-5 of 0 (t < 2^58 in magnitude), relative to about 2^-75:
 * s W with W = 1 - s/2 + s^2 S(s), S = sum of (-s)^m/(m + 3), m <= 12, by Horner's rule in y = 2^5 |s| in Q64, W in Q126
 */
static inline struct pow_float pow_log_near(uint64_t t, bool negative)
{
	uint64_t y = t << 6;
	struct pow_wide square = pow_wide_product(t, t);
	struct pow_wide w = { .hi = 0x4000000000000000ull, .lo = 0 };
	struct pow_wide half = { .hi = t >> 2, .lo = t << 62 };
	struct pow_wide term;
	uint64_t s;

	if (negative) {
		s = 0x0000000000000025ull + __libc_trig_high64(y, 0x0000000000000001ull);
		s = 0x00000000000004ecull + __libc_trig_high64(y, s);
		s = 0x000000000000aaabull + __libc_trig_high64(y, s);
		s = 0x00000000001745d1ull + __libc_trig_high64(y, s);
		s = 0x0000000003333333ull + __libc_trig_high64(y, s);
		s = 0x0000000071c71c72ull + __libc_trig_high64(y, s);
		s = 0x0000001000000000ull + __libc_trig_high64(y, s);
		s = 0x0000024924924925ull + __libc_trig_high64(y, s);
		s = 0x0000555555555555ull + __libc_trig_high64(y, s);
		s = 0x000ccccccccccccdull + __libc_trig_high64(y, s);
		s = 0x0200000000000000ull + __libc_trig_high64(y, s);
		s = 0x5555555555555555ull + __libc_trig_high64(y, s);
		w = pow_wide_add(w, half);
	} else {
		s = 0x0000000000000025ull - __libc_trig_high64(y, 0x0000000000000001ull);
		s = 0x00000000000004ecull - __libc_trig_high64(y, s);
		s = 0x000000000000aaabull - __libc_trig_high64(y, s);
		s = 0x00000000001745d1ull - __libc_trig_high64(y, s);
		s = 0x0000000003333333ull - __libc_trig_high64(y, s);
		s = 0x0000000071c71c72ull - __libc_trig_high64(y, s);
		s = 0x0000001000000000ull - __libc_trig_high64(y, s);
		s = 0x0000024924924925ull - __libc_trig_high64(y, s);
		s = 0x0000555555555555ull - __libc_trig_high64(y, s);
		s = 0x000ccccccccccccdull - __libc_trig_high64(y, s);
		s = 0x0200000000000000ull - __libc_trig_high64(y, s);
		s = 0x5555555555555555ull - __libc_trig_high64(y, s);
		w = pow_wide_add(w, pow_wide_negate(half));
	}
	/* s^2 S in Q126 is t^2 S 2^-64 (t^2 below 2^116) */
	term = pow_wide_product(square.lo, s);
	term = pow_wide_add((struct pow_wide){ .hi = 0, .lo = term.hi }, pow_wide_product(square.hi, s));
	w = pow_wide_add(w, term);

	return pow_float_product(pow_float_from((struct pow_wide){ .hi = 0, .lo = t }, 63), pow_float_from(w, 126));
}

/*
 * 2^(n + f 2^-64) for a result beyond the kernel's: from 1024 overflow, below -1075 underflow, between them a
 * subnormal rounded once from the kernel's 64 bits (__libc_dp_fixed_exp2_fraction) at its own last place, a carry
 * making it normal
 */
[[gnu::noinline]] static double pow_ends(int32_t n, uint64_t f, uint64_t sign)
{
	double s = sign != 0 ? -1.0 : 1.0;
	uint64_t y;
	int32_t shift;

	if (n >= 1024)
		return __libc_math_overflow(s);
	if (n < -1075)
		return __libc_math_underflow(s);

	/* (y 2^-63) 2^n as subnormal bits: y >> (11 + (-1022 - n)), with its rounding bit */
	y = __libc_dp_fixed_exp2_fraction(f);
	shift = -1011 - n;
	if (shift >= 64)
		return __libc_double_from(sign | (y >> 63));

	return __libc_double_from(sign | ((y >> shift) + ((y >> (shift - 1)) & 1)));
}

/*
 * The fast pow of the soft-double cores (PLAN D162, D194), in fixed point with one rounding (the kernel's): log2|x|
 * relative to about 2^-75, from the table path (ln z in Q126 times log2(e), plus k) or within 2^-5 of 1 from the
 * relative series; times y, both as normalized 96-bit products; then 2^(n + f) by the exponentials' kernel (D188). The
 * special cases that need no logarithm out of line (__libc_fast_pow_special, D193's); a negative x with an integer y
 * fixed in line, a subnormal x by the reduction. C23 7.12.7.5, F.10.4.5.
 */
double pow(double x, double y)
{
	uint64_t ix = __libc_double_bits(x);
	uint64_t iy = __libc_double_bits(y);
	uint64_t sign = 0;
	struct pow_float g;
	struct pow_float t;
	bool negative;
	uint64_t z;
	uint64_t f;
	int32_t k;
	int32_t n;

	if (((ix >> 52) - 0x001) >= 0x7fe || ((iy >> 52) & 0x7ff) - 0x3be >= 0x43e - 0x3be) {
		/* Here only a negative finite x or a subnormal one with y of 2^-65 to 2^63: the rest needs no logarithm */
		if (((iy >> 52) & 0x7ff) - 0x3be >= 0x43e - 0x3be || (ix << 1) == 0 || ((ix >> 52) & 0x7ff) == 0x7ff)
			return __libc_fast_pow_special(x, y);
		if ((ix >> 63) != 0) {
			int kind = __libc_fast_pow_integer_kind(iy);

			if (kind == 0)
				return __libc_fast_pow_special(x, y);
			sign = kind == 1 ? 0x8000000000000000ull : 0;
			ix &= 0x7fffffffffffffffull;
		}
	}

	z = __libc_dp_fixed_log_reduce(ix, &k);
	if (k == 0 && z - 0x7c00000000000001ull < 0x07ffffffffffffffull) {
		negative = z < 0x8000000000000000ull;
		if (z == 0x8000000000000000ull)
			return __libc_double_from(0x3ff0000000000000ull | sign);
		g = pow_float_product(pow_log_near(negative ? 0x8000000000000000ull - z : z - 0x8000000000000000ull, negative), pow_log2e);
	} else {
		struct pow_wide l = pow_log_table(z);
		struct pow_wide v;
		bool below = (int64_t)l.hi < 0;

		g = pow_float_product(pow_float_from(below ? pow_wide_negate(l) : l, 126), pow_log2e);
		/* k + ln z log2(e) in Q96 (m 2^(e + 1), e <= -1 as |ln z log2(e)| < 1), k above it, back to a pow_float */
		v = pow_wide_shift(pow_float_significand(g), -1 - g.e);
		if (below)
			v = pow_wide_negate(v);
		v.hi += (uint64_t)(int64_t)k << 32;
		negative = (int64_t)v.hi < 0;
		g = pow_float_from(negative ? pow_wide_negate(v) : v, 96);
	}

	/* y log2|x|, its sign y's times log2|x|'s */
	t = pow_float_product(g, (struct pow_float){ .hi = ((iy << 12) >> 1) | 0x8000000000000000ull, .lo = 0, .e = (int32_t)((iy >> 52) & 0x7ff) - 1023 });
	negative = negative != ((iy >> 63) != 0);
	/* From 2^10 the result overflows; below -2^10 it is subnormal down to -1075 (pow_ends), beyond 2^11 it underflows */
	if (t.e > 10 || (t.e == 10 && !negative))
		return pow_ends(negative ? -2000 : 2000, 0, sign);

	/* As n + f 2^-64: t 2^64 = m 2^(e - 31), m of 96 bits, e <= 10 */
	{
		struct pow_wide v = pow_wide_shift(pow_float_significand(t), 31 - t.e);

		n = (int32_t)v.hi;
		f = v.lo;
	}
	if (negative)
		f = __libc_dp_fixed_negate(&n, f);
	if (n < -1022 || n > 1023)
		return pow_ends(n, f, sign);

	return __libc_double_from(__libc_double_bits(__libc_dp_fixed_exp2(n, f)) | sign);
}
