/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* 16^-k/(2k + 1)! in Q64 for k = 1..6, the coefficients of w^k for w = 16 x^2 (the seventh is below 2^-68) */
#define C1 0x02aaaaaaaaaaaaabull
#define C2 0x0002222222222222ull
#define C3 0x000000d00d00d00dull
#define C4 0x000000002e3bc74bull
#define C5 0x000000000006b991ull
#define C6 0x00000000000000b1ull

/* The ends: +-0, +-inf and NaN return themselves, below 2^-27 x nudged away from zero (underflow for a subnormal), above ln(2 DBL_MAX) overflow */
[[gnu::noinline]] static double sinh_ends(double x)
{
	if (isnan(x) || isinf(x) || x == 0)
		return x + x;
	if (__builtin_fabs(x) < 0x1p-27)
		return __libc_math_tiny(x, x);

	return __libc_math_overflow(__builtin_copysign(1.0, x));
}

/*
 * The fast sinh of the soft-double cores (PLAN D162, D202), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, 2^-27 <= |x| <= 0x1.633ce8fb9f87dp+9:
 * - below 1/4, x (1 + V) with V = x^2/6 + x^4/120 + ... in Q64 from w = 16 x^2 (Horner's rule, unsigned), relative
 *   however small x is;
 * - from 1/4, (e^|x| - e^-|x|)/2 from exp's kernel twice on one scale (__libc_dp_fixed_exp_pair), the difference exact
 *   in integers: the result is at least 0.25 and its relative error a few units of 2^-61.
 * C23 7.12.5.5, F.10.2.5.
 */
double sinh(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t minus;
	uint64_t plus;
	uint64_t m;
	uint64_t w;
	uint64_t v;
	int32_t n;

	/* 2^-27 <= |x| < 1024, and the overflow threshold */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x409) || magnitude > 0x408633ce8fb9f87dull)
		return sinh_ends(x);

	if (e < -2) {
		/* M = m 2^-63 in [1, 2), x^2 = M^2 2^2e, M^2 in Q62 = high(m, m): w = 16 x^2 in Q64 */
		m = ((magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
		w = __libc_trig_high64(m, m) >> (-2 * e - 6);
		v = C5 + __libc_trig_high64(w, C6);
		v = C4 + __libc_trig_high64(w, v);
		v = C3 + __libc_trig_high64(w, v);
		v = C2 + __libc_trig_high64(w, v);
		v = C1 + __libc_trig_high64(w, v);
		v = __libc_trig_high64(w, v);
		/* M (1 + V) in Q62 */
		return __libc_dp_fixed_normal(sign, e + 1, __libc_trig_high64(m, 0x8000000000000000ull + (v >> 1)));
	}

	/* sinh |x| = 2^(n - 1) (Y1 - Y2') 2^-63 */
	plus = __libc_dp_fixed_exp_pair(magnitude, &minus, &n);
	return __libc_dp_fixed_normal(sign, n - 1, plus - minus);
}
