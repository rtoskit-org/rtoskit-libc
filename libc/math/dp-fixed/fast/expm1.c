/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expm1.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* (16 |x|)^k/(k + 1)! 2^-4k in Q64 for k = 1..9; the tenth is below 0.43 units of 2^-64 */
#define C1 0x0800000000000000ull
#define C2 0x002aaaaaaaaaaaabull
#define C3 0x0000aaaaaaaaaaabull
#define C4 0x0000022222222222ull
#define C5 0x00000005b05b05b0ull
#define C6 0x000000000d00d00dull
#define C7 0x00000000001a01a0ull
#define C8 0x0000000000002e3cull
#define C9 0x000000000000004aull

/*
 * The ends (C23 7.12.6.6, F.10.3.4, and the strict file): NaN, below 2^-54 x nudged up (underflow for a subnormal), from
 * 512 exp's slow path (the 1 is below 2^-500 of the result; overflow and +inf), to -512 and -inf -1 + 2^-100 (inexact,
 * rounding in every direction at run time: the fast files drop -frounding-math)
 */
[[gnu::noinline]] static double expm1_special(double x)
{
	volatile double tiny = 0x1p-100;

	if (isnan(x))
		return x + x;
	if (__builtin_fabs(x) < 0x1p-54)
		return __libc_math_tiny(x, 1.0);
	if (x > 0)
		return __libc_fast_exp_special(x);
	if (isinf(x))
		return -1.0;

	return tiny - 1.0;
}

/* The double of 2^(e - 63) z for 2^57 <= z < 2^64 (z normalized here), rounded to nearest by its 54th bit, with sign */
static inline double expm1_result(uint64_t sign, int32_t e, uint64_t z)
{
	while (z < 0x8000000000000000ull) {
		z <<= 1;
		e--;
	}

	return __libc_double_from(sign | ((((uint64_t)(int64_t)(e + 1022) << 52) + (z >> 11) + ((z >> 10) & 1))));
}

/*
 * The fast expm1 of the soft-double cores (PLAN D162, D189), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, 2^-54 <= |x| < 512:
 * - below 1/16, x (1 + q) with q = x/2 + x^2/6 + ... in Q64 from y = 16 |x| (Horner's rule, unsigned, subtracting for
 *   x < 0; the four inner steps' operands fit in 32 bits), relative however small x is;
 * - from 1/16, 2^t from exp's kernel in Q63 (__libc_dp_fixed_exp2_fraction), t = x log2(e), minus 1 exactly in integers:
 *   the result is then at least 0.06, so the kernel's few units of 2^-63 stay below 0.04 ulp; below -40 e^x is below
 *   2^-57 and the result -1 + 2^-100.
 */
double expm1(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t m = ((bits & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
	int32_t e = (int32_t)((bits >> 52) & 0x7ff) - 1023;
	uint64_t y;
	uint64_t p;
	uint64_t f;
	int32_t n;

	if (__libc_fast_exp_outside(x, 0x3c9, 0x408))
		return expm1_special(x);

	if (e < -4) {
		y = m >> (-e - 5);
		if (sign == 0) {
			p = C8 + __libc_trig_high64(y, C9);
			p = C7 + __libc_trig_high64(y, p);
			p = C6 + __libc_trig_high64(y, p);
			p = C5 + __libc_trig_high64(y, p);
			p = C4 + __libc_trig_high64(y, p);
			p = C3 + __libc_trig_high64(y, p);
			p = C2 + __libc_trig_high64(y, p);
			p = C1 + __libc_trig_high64(y, p);
			p = 0x8000000000000000ull + (__libc_trig_high64(y, p) >> 1);
		} else {
			p = C8 - __libc_trig_high64(y, C9);
			p = C7 - __libc_trig_high64(y, p);
			p = C6 - __libc_trig_high64(y, p);
			p = C5 - __libc_trig_high64(y, p);
			p = C4 - __libc_trig_high64(y, p);
			p = C3 - __libc_trig_high64(y, p);
			p = C2 - __libc_trig_high64(y, p);
			p = C1 - __libc_trig_high64(y, p);
			p = 0x8000000000000000ull - (__libc_trig_high64(y, p) >> 1);
		}
		/* x (1 + q) = m 2^(e - 63) p 2^-63, so 2^(e + 1 - 63) high */
		return expm1_result(sign, e + 1, __libc_trig_high64(m, p));
	}

	/* Below -40 (the bits of a negative double grow with its magnitude) */
	if (bits > 0xc044000000000000ull)
		return expm1_special(x);
	f = __libc_dp_fixed_scaled(m >> 11, e, 0xb8aa3b295c17f0bbull, 0xbe87fed0691d3e88ull, 63, &n);
	if (sign == 0) {
		y = __libc_dp_fixed_exp2_fraction(f);
		/* 2^(n - 63) y - 1: from 2^64 the 1 is below a unit of y */
		if (n < 64)
			y -= 0x8000000000000000ull >> n;
		return expm1_result(0, n, y);
	}

	f = __libc_dp_fixed_negate(&n, f);
	/* 1 - 2^(n - 63) y, n <= -1 */
	y = __libc_dp_fixed_exp2_fraction(f);
	return expm1_result(sign, 0, 0x8000000000000000ull - (-n < 64 ? y >> -n : 0));
}
