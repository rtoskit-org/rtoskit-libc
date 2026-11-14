/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* The ends: +-0, +-inf and NaN return themselves, below 2^-27 x nudged toward zero (underflow for a subnormal), from 2^28 log|x| + ln 2 */
[[gnu::noinline]] static double asinh_ends(double x)
{
	double hi;
	double lo;

	if (isnan(x) || isinf(x) || x == 0)
		return x + x;
	if (__builtin_fabs(x) < 0x1p-27)
		return __libc_math_tiny(x, -x);

	/* sqrt(x^2 + 1) = |x| (1 + 1/(2 x^2)): the term 1/(4 x^2) is below 2^-58 */
	hi = __libc_fast_log_pair(__builtin_fabs(x), 0.0, 1, &lo);
	return __builtin_copysign(hi + lo, x);
}

/*
 * The fast asinh of the double-FPU cores (PLAN D162, D201), in double with one rounding: log(|x| + sqrt(x^2 + 1)) with
 * the argument as an exact-enough pair, so no series near 0: x^2 exact (fma or Dekker), 1 + x^2 by a two-sum, the square
 * root with its remainder (__libc_fast_sqrt_pair), |x| + s by a fast two-sum (s > |x|), and the logarithm of the pair
 * (__libc_fast_log_pair), relative however small it is. From 2^28 log|x| + ln 2. C23 7.12.5.2, F.10.2.2.
 */
double asinh(double x)
{
	double a = __builtin_fabs(x);
	struct __libc_dd square;
	struct __libc_dd sum;
	struct __libc_dd v;
	double s_lo;
	double hi;
	double lo;
	double s;

	/* 2^-27 <= |x| < 2^28 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x41b))
		return asinh_ends(x);

	square = __libc_dd_product(a, a);
	sum = __libc_dd_sum(1.0, square.hi);
	s = __libc_fast_sqrt_pair(sum.hi, sum.lo + square.lo, &s_lo);
	v = __libc_dd_fast_sum(s, a);
	hi = __libc_fast_log_pair(v.hi, v.lo + s_lo, 0, &lo);

	return __builtin_copysign(hi + lo, x);
}
