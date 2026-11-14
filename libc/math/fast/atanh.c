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
 * The fast atanh of the double-FPU cores (PLAN D162, D201), in double with one rounding: log((1 + |x|)/(1 - |x|))/2 with
 * 1 + |x| and 1 - |x| as exact pairs (fast two-sums), their quotient with its exact remainder and the low parts, and the
 * logarithm of the pair (__libc_fast_log_pair), relative however small it is: no series near 0. C23 7.12.5.3, F.10.2.3.
 */
double atanh(double x)
{
	double a = __builtin_fabs(x);
	struct __libc_dd n;
	struct __libc_dd d;
	double hi;
	double lo;
	double q;

	/* 2^-27 <= |x| < 1 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x3ff))
		return atanh_ends(x);

	n = __libc_dd_fast_sum(1.0, a);
	d = __libc_dd_fast_sum(1.0, -a);
	q = __libc_fast_quotient(n.hi, d.hi, &lo);
	lo += (n.lo - q * d.lo) / d.hi;
	hi = __libc_fast_log_pair(q, lo, 0, &lo);

	return __builtin_copysign(0.5 * (hi + lo), x);
}
