/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp10_special.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The slow path of the fast exp10 (libc/math/fast/exp10.c and libc/math/dp-fixed/fast/exp10.c, PLAN D187, D188): tiny
 * arguments, the ends, the infinities and NaN, by the strict file's double-double core (rare; the core is shared with pow)
 */
double __libc_fast_exp10_special(double x)
{
	struct __libc_dd value;
	struct __libc_dd z;
	int exponent;

	if (isnan(x))
		return x + x;
	/* 10^x rounds as 1 + x ln 10 (1 + x alone was 1.15 ulp below 1, measured) */
	if (__builtin_fabs(x) < 0x1p-54)
		return 1.0 + x * 0x1.26bb1bbb55516p+1;
	if (x > 0x1.34413509f79fep+8)
		return isinf(x) ? x : __libc_math_overflow(1.0);
	if (x < -0x1.439b746e36b52p+8)
		return isinf(x) ? 0.0 : __libc_math_underflow(1.0);
	z = __libc_dd_product(x, __LIBC_LN10.hi);
	z = __libc_dd_fast_sum(z.hi, z.lo + x * __LIBC_LN10.lo);
	value = __libc_exp_core(z.hi, z.lo, &exponent);
	return __libc_exp_scale(value, exponent);
}
