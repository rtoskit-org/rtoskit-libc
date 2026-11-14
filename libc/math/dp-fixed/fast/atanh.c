/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* The ends: +-0 and NaN return themselves, beyond 1 a domain error, +-1 a pole, below 2^-27 x nudged away from zero */
[[gnu::noinline]] static double atanh_ends(double x)
{
	double a = __builtin_fabs(x);

	if (isnan(x) || x == 0)
		return x + x;
	if (a > 1.0)
		return __libc_math_invalid();
	if (a == 1.0)
		return __libc_math_divbyzero(__builtin_copysign(1.0, x));

	return __libc_math_tiny(x, x);
}

/*
 * The fast atanh of the soft-double cores (PLAN D162, D203), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, 2^-27 <= |x| < 1: log1p(s)/2 with s = 2|x|/(1 - |x|), 1 - |x| in Q64 (exact from
 * 2^-12, else to a unit), the quotient by the 64-bit reciprocal (to about 2^-62 relatively), and log1p of it (relative
 * below 2^-5); the halving exact in the result's exponent. C23 7.12.5.3, F.10.2.3.
 */
double atanh(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	int32_t e = (int32_t)((bits ^ sign) >> 52) - 1023;
	uint64_t a = ((bits & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
	uint64_t d;
	uint64_t q;
	int32_t k;

	/* 2^-27 <= |x| < 1 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x3ff))
		return atanh_ends(x);

	/* (1 - |x|) 2^64 modulo 2^64, |x| 2^64 = M 2^(e + 64) */
	d = 0 - (a >> (-e - 1));
	k = __builtin_clzll(d);
	/* s = (M/((1 - |x|) 2^k)) 2^(e + 2 + k) = q 2^-63 2^(e + 2 + k) */
	q = __libc_trig_high64(a, __libc_dp_fixed_reciprocal(d << k));
	k += e + 2 - __builtin_clzll(q);
	q <<= __builtin_clzll(q);

	return __libc_double_from((__libc_double_bits(__libc_dp_fixed_log1p_scaled(q, k)) - 0x0010000000000000ull) | sign);
}
