/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cos.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast cos of the soft-double cores (PLAN D162, D197; docs/research/libm-fast-double.md, section 15): the speed
 * file's entry tests and D136's fixed-point kernel, rounded to nearest with no rounding test; the double-double path only
 * where the kernel declines. C23 7.12.4.5, F.10.1.5.
 */
double cos(double x)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed value;
	double result;

	/* cos.c's entry tests on the bits: NaN, infinity, zero, |x| < 2^-27 */
	if (a >= __LIBC_TRIG_FIXED_INF)
		return a > __LIBC_TRIG_FIXED_INF ? x + x : __libc_math_invalid();
	if (a == 0)
		return 1.0;
	if (a < __LIBC_TRIG_FIXED_TINY) {
		volatile double tiny = 0x1p-80;

		return 1.0 - tiny;
	}

	if (__libc_trig_fixed(x, __LIBC_TRIG_FIXED_COS, nullptr, &value))
		return __libc_dp_fixed_trig_round(&value);

	__libc_trig_dd(x, __LIBC_TRIG_FIXED_COS, nullptr, &result);
	return result;
}
