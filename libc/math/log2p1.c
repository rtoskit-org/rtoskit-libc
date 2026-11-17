/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2p1.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/* C23 7.12.6.17: log1p(-1) is -infinity (divide-by-zero), below -1 a domain error */
double log2p1(double x)
{
	struct __libc_dd log;

	if (isnan(x))
		return x + x;
	if (x == -1.0)
		return __libc_math_divbyzero(-1.0);
	if (x < -1.0)
		return __libc_math_invalid();
	/*
	 * From 2^100 (infinity included) the core's correction 1/x is below 2^-54 ulp of the result, and near DBL_MAX it
	 * is subnormal and would raise an undeserved underflow (PLAN D227): log2p1(x) is log2(x)
	 */
	if (x > 0x1p100)
		return log2(x);
	if (fabs(x) < 0x1p-60)
		return x * __LIBC_INV_LN2.hi;

	log = __libc_log1p_core(x);
	log = __libc_dd_multiply(log, __LIBC_INV_LN2);
	return log.hi + log.lo;
}
