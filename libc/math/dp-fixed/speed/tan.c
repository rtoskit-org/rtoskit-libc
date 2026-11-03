/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tan.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "trig-fixed.h"

/*
 * C23 7.12.4.7 in the speed flavour (P9.03, Q42): tan.c's entry tests on the
 * bits, then sin x / cos x from the fixed-point kernel, correctly rounded
 * when its rounding test passes, else tan.c's double-double path
 */
double tan(double x)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed value;
	double result;

	if (a >= __LIBC_TRIG_FIXED_INF)
		return a > __LIBC_TRIG_FIXED_INF ? x + x : __libc_math_invalid();
	if (a < __LIBC_TRIG_FIXED_TINY)
		return __libc_math_tiny(x, x);

	if (__libc_tan_fixed(x, &value) && __libc_trig_fixed_round(&value, __LIBC_TAN_FIXED_ERROR, &result))
		return result;

	return __libc_tan_dd(x);
}
