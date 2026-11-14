/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acosh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* The ends: NaN returns itself, below 1 (and -inf) a domain error, +inf itself, from 2^28 log x + ln 2 */
[[gnu::noinline]] static double acosh_ends(double x)
{
	double hi;
	double lo;

	if (isnan(x))
		return x + x;
	if (x < 1.0)
		return __libc_math_invalid();
	if (isinf(x))
		return x;

	/* sqrt(x^2 - 1) = x (1 - 1/(2 x^2)): the term -1/(4 x^2) is below 2^-58 */
	hi = __libc_fast_log_pair(x, 0.0, 1, &lo);
	return hi + lo;
}

/*
 * The fast acosh of the double-FPU cores (PLAN D162, D201), in double with one rounding: log(x + sqrt(x^2 - 1)) with the
 * argument as a pair, so no series near 1: x^2 exact (fma or Dekker), x^2 - 1 exact up to 2 (Sterbenz) and by a fast
 * two-sum above, normalized, the square root with its remainder, x + s by a fast two-sum (s < x), and the logarithm of
 * the pair (__libc_fast_log_pair), relative however close x is to 1 (the result is at least 2^-25.5). acosh(1) is +0.
 * From 2^28 log x + ln 2. C23 7.12.5.1, F.10.2.1.
 */
double acosh(double x)
{
	struct __libc_dd square;
	struct __libc_dd t;
	struct __libc_dd v;
	double s_lo;
	double hi;
	double lo;
	double s;

	/* 1 < x < 2^28; 1 itself is +0 in every rounding direction */
	/* Quiet comparisons, so that a quiet NaN raises nothing */
	if (!(isgreater(x, 1.0) && isless(x, 0x1p28))) {
		if (x == 1.0)
			return 0.0;
		return acosh_ends(x);
	}

	square = __libc_dd_product(x, x);
	t = __libc_dd_fast_sum(square.hi, -1.0);
	t = __libc_dd_fast_sum(t.hi, t.lo + square.lo);
	s = __libc_fast_sqrt_pair(t.hi, t.lo, &s_lo);
	v = __libc_dd_fast_sum(x, s);
	hi = __libc_fast_log_pair(v.hi, v.lo + s_lo, 0, &lo);

	return hi + lo;
}
