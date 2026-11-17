/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_tgamma_ends.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The ends (C23 7.12.8.4, F.10.5.4): NaN returns itself, +inf +inf, -inf invalid, +-0 +-inf with divide-by-zero; below
 * 2^-54 in magnitude 1/x - gamma (as the strict file: gamma decides only the directed roundings); from 171.62437695630272 the overflow; |x| from 2^52
 * is an integer: the overflow, or invalid when negative
 */
double __libc_fast_tgamma_ends(double x)
{
	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? x : __libc_math_invalid();
	if (x == 0)
		return __libc_math_divbyzero(__builtin_copysign(1.0, x));
	if (__builtin_fabs(x) < 0x1p-54) {
		/*
		 * 1/x - gamma (Euler's constant) as the strict file forms it: gamma is below half an ulp of 1/x, so it decides
		 * only the directed roundings, and it is lost if 1/x is rounded first. 1/x as a pair, to nearest, on x scaled by
		 * 2^64, then rounded once in the caller's direction; at or below 2^-1024 1/x overflows alike.
		 */
		struct __libc_dd reciprocal;
		int mode;

		if (__builtin_fabs(x) <= 0x1p-1024)
			return 1.0 / x;
		mode = __libc_math_nearest();
		reciprocal = __libc_dd_divide((struct __libc_dd){ .hi = 1.0, .lo = 0.0 }, (struct __libc_dd){ .hi = x * 0x1p64, .lo = 0.0 });
		reciprocal.lo -= 0x1.2788cfc6fb619p-1 * 0x1p-64;
		__libc_math_restore(mode);
		return reciprocal.hi * 0x1p64 + reciprocal.lo * 0x1p64;
	}
	if (x < 0)
		return __libc_math_invalid();

	return __libc_math_overflow(1.0);
}
