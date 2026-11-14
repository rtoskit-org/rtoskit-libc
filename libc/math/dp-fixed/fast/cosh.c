/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* The ends: NaN returns itself and +-inf +inf, below 2^-27 1 + tiny (inexact; cosh(+-0) is 1), above ln(2 DBL_MAX) overflow */
[[gnu::noinline]] static double cosh_ends(double x)
{
	volatile double tiny = 0x1p-80;
	double a = __builtin_fabs(x);

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return a;
	if (a < 0x1p-27)
		return x == 0 ? 1.0 : 1.0 + tiny;

	return __libc_math_overflow(1.0);
}

/*
 * The fast cosh of the soft-double cores (PLAN D162, D202), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path: (e^|x| + e^-|x|)/2 from exp's kernel twice on one scale (__libc_dp_fixed_exp_pair), the
 * sum exact in 65 bits; nothing cancels, so no series near 0. 2^-27 <= |x| <= 0x1.633ce8fb9f87dp+9. C23 7.12.5.4,
 * F.10.2.4.
 */
double cosh(double x)
{
	uint64_t magnitude = __libc_double_bits(x) & 0x7fffffffffffffffull;
	uint64_t minus;
	uint64_t plus;
	uint64_t sum;
	int32_t n;

	/* 2^-27 <= |x| < 1024, and the overflow threshold */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x409) || magnitude > 0x408633ce8fb9f87dull)
		return cosh_ends(x);

	/* cosh |x| = 2^(n - 1) (Y1 + Y2') 2^-63, the sum's carry (from 2^64) into the exponent */
	plus = __libc_dp_fixed_exp_pair(magnitude, &minus, &n);
	sum = plus + minus;
	if (sum < plus)
		return __libc_dp_fixed_double(0, n, (sum >> 1) | 0x8000000000000000ull);

	return __libc_dp_fixed_double(0, n - 1, sum);
}
