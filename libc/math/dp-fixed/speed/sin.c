/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sin.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "trig-fixed.h"

/*
 * C23 7.12.4.6 in the speed flavour (P8.03, PLAN D133): sin.c's entry tests,
 * then the fixed-point kernel, correctly rounded when its rounding test
 * passes, else the double-double path
 */
double sin(double x)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed value;
	double result;

	/* sin.c's entry tests on the bits: NaN, infinity, |x| < 2^-27 */
	if (a >= __LIBC_TRIG_FIXED_INF)
		return a > __LIBC_TRIG_FIXED_INF ? x + x : __libc_math_invalid();
	if (a < __LIBC_TRIG_FIXED_TINY)
		return __libc_math_tiny(x, -x);

	if (__libc_trig_fixed(x, __LIBC_TRIG_FIXED_SIN, &value, nullptr) && __libc_trig_fixed_round(&value, __LIBC_TRIG_FIXED_ERROR, &result))
		return result;

	__libc_trig_dd(x, __LIBC_TRIG_FIXED_SIN, &result, nullptr);
	return result;
}
