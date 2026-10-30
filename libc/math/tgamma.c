/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgamma.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * C23 7.12.8.4, F.10.5.4: tgamma(+-0) is +-infinity with divide-by-zero, a
 * negative integer or -infinity a domain error. Factorials up to 22! are
 * exact products; otherwise exp(lgamma) with the sign of the reflection,
 * scaled once (so overflow and subnormal results round once).
 */
double tgamma(double x)
{
	struct __libc_dd value;
	int sign;
	int exponent;
	int mode;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? x : __libc_math_invalid();
	if (x == 0)
		return __libc_math_divbyzero(copysign(1.0, x));
	if (x < 0 && x == floor(x))
		return __libc_math_invalid();
	if (x > 171.62437695630272)
		return __libc_math_overflow(1.0);
	if (x == floor(x) && x <= 23.0) {
		double product = 1.0;

		for (int k = 2; k < (int)x; ++k)
			product *= k;
		return product;
	}
	if (fabs(x) < 0x1p-54) {
		/*
		 * 1/x - gamma (Euler's constant): gamma is below half an ulp of 1/x,
		 * so it decides only the directed roundings, and it is lost if 1/x is
		 * rounded first. So 1/x is taken as a double-double, in round to
		 * nearest, on x scaled by 2^64 (normal, and Dekker's split of the
		 * quotient stays finite), and the whole is rounded once in the
		 * caller's direction. At or below 2^-1024 the result overflows,
		 * which 1/x does in the same way.
		 */
		struct __libc_dd reciprocal;

		if (fabs(x) <= 0x1p-1024)
			return 1.0 / x;
		mode = __libc_math_nearest();
		reciprocal = __libc_dd_divide((struct __libc_dd){ .hi = 1.0, .lo = 0.0 }, (struct __libc_dd){ .hi = x * 0x1p64, .lo = 0.0 });
		reciprocal.lo -= 0x1.2788cfc6fb619p-1 * 0x1p-64;
		__libc_math_restore(mode);
		return reciprocal.hi * 0x1p64 + reciprocal.lo * 0x1p64;
	}

	mode = __libc_math_nearest();
	value = __libc_lgamma_core(x, &sign);
	if (value.hi < -745.2) {
		__libc_math_restore(mode);
		return __libc_math_underflow(sign);
	}
	value = __libc_exp_core(value.hi, value.lo, &exponent);
	if (sign < 0)
		value = __libc_dd_negate(value);
	__libc_math_restore(mode);

	return __libc_exp_scale(value, exponent);
}
