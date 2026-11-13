/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast tan of the soft-double cores (PLAN D162, D197; docs/research/libm-fast-double.md, section 15): tan.c's entry
 * tests on the bits, then sin x / cos x from D142's fixed-point kernel, rounded to nearest with no rounding test;
 * tan.c's double-double path where the kernel declines. C23 7.12.4.7, F.10.1.7.
 */
double tan(double x)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed value;

	if (a >= __LIBC_TRIG_FIXED_INF)
		return a > __LIBC_TRIG_FIXED_INF ? x + x : __libc_math_invalid();
	if (a < __LIBC_TRIG_FIXED_TINY)
		return __libc_math_tiny(x, x);

	if (__libc_tan_fixed(x, &value))
		return __libc_dp_fixed_trig_round(&value);

	return __libc_tan_dd(x);
}
