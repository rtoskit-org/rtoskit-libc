/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sin.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast sin of the soft-double cores (PLAN D162, D197; docs/research/libm-fast-double.md, section 15): the speed
 * file's entry tests and D136's fixed-point kernel, rounded to nearest with no rounding test; the double-double path only
 * where the kernel declines (|x| >= 2^20, or a reduction that cancels more than 64 bits). C23 7.12.4.6, F.10.1.6.
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

	if (__libc_trig_fixed(x, __LIBC_TRIG_FIXED_SIN, &value, nullptr))
		return __libc_dp_fixed_trig_round(&value);

	__libc_trig_dd(x, __LIBC_TRIG_FIXED_SIN, &result, nullptr);
	return result;
}
