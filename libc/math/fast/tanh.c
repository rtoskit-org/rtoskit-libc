/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* The ends: +-0 and NaN return themselves, +-inf +-1; below 2^-27 x nudged toward zero (underflow for a subnormal); from 22 +-(1 - tiny) */
[[gnu::noinline]] static double tanh_ends(double x)
{
	volatile double tiny = 0x1p-80;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __builtin_copysign(1.0, x);
	if (__builtin_fabs(x) < 0x1p-27)
		return __libc_math_tiny(x, -x);

	/* At run time, so that it is inexact and rounds in every direction */
	return __builtin_copysign(1.0 - tiny, x);
}

/*
 * The fast tanh of the double-FPU cores (PLAN D162, D201), in double with one rounding. Below 1/4, x - x^3/3 + x^5 Q(z),
 * z = x^2, with 1/3 as a pair and Q of seven terms fitted here (2^-61.0 relative), by __libc_fast_odd_series. From 1/4 to 22, sinh |x| / cosh |x| from the hyperbolic kernel,
 * both as normalized pairs, the quotient with its exact remainder and the pairs' low parts. From 22, +-1 (inexact).
 * C23 7.12.5.6, F.10.2.6.
 */
double tanh(double x)
{
	double a = __builtin_fabs(x);
	struct __libc_dd n;
	struct __libc_dd d;
	double s_hi;
	double s_lo;
	double c_hi;
	double c_lo;
	double lo;
	double q;
	double z;

	/* 2^-27 <= |x| < 32 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x404) || a >= 22.0)
		return tanh_ends(x);
	if (a < 0.25) {
		z = x * x;
		q = __libc_fast_odd_series(x, -0x1.5555555555555p-2, -0x1.5555555555555p-56, 0x1.111111111110ep-3 + z * (-0x1.ba1ba1ba16291p-5 + z * (0x1.664f4877c8935p-6 + z * (-0x1.226e2d1c7317dp-7 + z * (0x1.d6cde2b292803p-9 + z * (-0x1.7c8a87d5d97b0p-10 + z * 0x1.1b588de24b4d6p-11))))), &lo);
		return q + lo;
	}

	__libc_fast_sinhcosh(a, 0, &s_hi, &s_lo, &c_hi, &c_lo);
	n = __libc_dd_fast_sum(s_hi, s_lo);
	d = __libc_dd_fast_sum(c_hi, c_lo);
	q = __libc_fast_quotient(n.hi, d.hi, &lo);
	lo += (n.lo - q * d.lo) / d.hi;

	return __builtin_copysign(q + lo, x);
}
