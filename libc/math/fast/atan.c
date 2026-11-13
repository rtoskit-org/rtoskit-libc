/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast atan of the double-FPU cores (PLAN D162, D198; docs/research/libm-fast-double.md, section 16): the table
 * kernel for |x| <= 1, and pi/2 - atan(1/|x|) with the reciprocal's remainder above; one rounding. C23 7.12.4.3,
 * F.10.1.3: atan(+-inf) is +-pi/2, as is every |x| from 2^60 once rounded; +-0 returns itself.
 */
double atan(double x)
{
	double a = __builtin_fabs(x);
	double hi;
	double lo;

	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (isless(a, 0x1p-27))
		return __libc_math_tiny(x, -x);
	if (!isless(a, 0x1p60)) {
		if (isnan(x))
			return x + x;
		return __libc_fast_signed_sum(__LIBC_PI_2.hi, __LIBC_PI_2.lo, signbit(x) != 0);
	}

	if (a <= 1.0) {
		hi = __libc_fast_atan_kernel(a, 0.0, &lo);
	} else {
		double u_lo;
		double u = __libc_fast_quotient(1.0, a, &u_lo);
		double k_lo;
		double k = __libc_fast_atan_kernel(u, u_lo, &k_lo);
		struct __libc_dd sum = __libc_dd_fast_sum(__LIBC_PI_2.hi, -k);

		hi = sum.hi;
		lo = (sum.lo + __LIBC_PI_2.lo) - k_lo;
	}

	return __libc_fast_signed_sum(hi, lo, signbit(x) != 0);
}
