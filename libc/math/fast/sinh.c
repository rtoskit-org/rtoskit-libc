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

#include "internal/math-fast.h"

/* The ends: +-0, +-inf and NaN return themselves, below 2^-27 x nudged away from zero (underflow for a subnormal), beyond 22 e^|x|/2 */
[[gnu::noinline]] static double sinh_ends(double x)
{
	double a = __builtin_fabs(x);

	if (isnan(x) || isinf(x) || x == 0)
		return x + x;
	if (a < 0x1p-27)
		return __libc_math_tiny(x, x);

	return __libc_fast_exp_half(a, __builtin_copysign(1.0, x));
}

/*
 * The fast sinh of the double-FPU cores (PLAN D162, D201), in double with one rounding. Below 1/2, x + x^3/6 + x^5 Q(z),
 * z = x^2, with 1/6 as a pair and Q of five terms fitted here (2^-63.4 relative), by __libc_fast_odd_series. From 1/2 to 22, (e^|x| - e^-|x|)/2 by the hyperbolic kernel
 * (__libc_fast_sinhcosh, the halving in the scales): the difference of the scales exact, the rest in the low part, whose
 * rounding errors are below 2^-58 of the result from 1/2 (u is at most ln 2/128). Beyond, e^|x|/2 (overflow above
 * ln(2 DBL_MAX)). C23 7.12.5.5, F.10.2.5.
 */
double sinh(double x)
{
	double a = __builtin_fabs(x);
	double s_hi;
	double s_lo;
	double c_hi;
	double c_lo;
	double hi;
	double lo;
	double z;

	/* 2^-27 <= |x| < 32 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x404) || a > 22.0)
		return sinh_ends(x);
	if (a < 0.5) {
		z = x * x;
		hi = __libc_fast_odd_series(x, 0x1.5555555555555p-3, 0x1.5555555555555p-57, 0x1.1111111111112p-7 + z * (0x1.a01a01a0175f5p-13 + z * (0x1.71de3a6a7deb5p-19 + z * (0x1.ae63e08b88574p-26 + z * 0x1.6231d951fe746p-33))), &lo);
		return hi + lo;
	}

	__libc_fast_sinhcosh(a, 1, &s_hi, &s_lo, &c_hi, &c_lo);
	return __builtin_copysign(s_hi + s_lo, x);
}
