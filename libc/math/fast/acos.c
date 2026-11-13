/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acos.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast acos of the double-FPU cores (PLAN D162, D198; docs/research/libm-fast-double.md, section 16): pi/2 - asin x
 * up to 1/2, and 2 asin(sqrt((1 - |x|)/2)) or pi minus it above, with the square root's remainder; one rounding.
 * C23 7.12.4.1, F.10.1.1: acos(1) is +0; |x| > 1 is a domain error.
 */
double acos(double x)
{
	double a = __builtin_fabs(x);
	struct __libc_dd first;
	struct __libc_dd second;
	double rest;
	double t_lo;
	double t;
	double s;

	/* A NaN compares quietly (islessequal, isless): the relational operators would raise invalid */
	if (islessequal(a, 0.5)) {
		/* pi/2 - (x + x w), |x| <= 1/2 < pi/2: z = x^2 as an exact pair, x w_hi exact, two fast two-sums */
		struct __libc_dd z = __libc_dd_product(x, x);

		t = __libc_fast_asin_series(x, z.hi, z.lo, &t_lo);
		first = __libc_dd_fast_sum(__LIBC_PI_2.hi, -x);
		second = __libc_dd_fast_sum(first.hi, -t);
		return second.hi + (((second.lo + first.lo) + __LIBC_PI_2.lo) - t_lo);
	}
	if (!isless(a, 1.0)) {
		if (x == 1.0)
			return 0.0;
		if (x == -1.0)
			return __LIBC_PI.hi + __LIBC_PI.lo;
		return isnan(x) ? x + x : __libc_math_invalid();
	}

	s = __libc_fast_asin_half(a, &rest, &t_lo, &t);
	if (x > 0) {
		/* 2 asin s, relative however close x is to 1 */
		first = __libc_dd_fast_sum(2.0 * s, 2.0 * t);
		return first.hi + (first.lo + 2.0 * (rest + t_lo));
	}
	first = __libc_dd_fast_sum(__LIBC_PI.hi, -2.0 * s);
	second = __libc_dd_fast_sum(first.hi, -2.0 * t);

	return second.hi + (((second.lo + first.lo) + __LIBC_PI.lo) - 2.0 * (rest + t_lo));
}
