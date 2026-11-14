/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_pow_special.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The special cases (C23 7.12.7.5, F.10.4.5, and the strict file) that need no logarithm: y = +-0, x = 1, NaN, an
 * infinite y, x = +-0 or +-inf, x < 0 with y not an integer, and |y| below 2^-65 or from 2^63 for a finite nonzero x
 */
double __libc_fast_pow_special(double x, double y)
{
	uint64_t iy = __libc_double_bits(y);
	double sign;
	int kind;

	if ((iy << 1) == 0 || x == 1.0)
		return 1.0;
	if (isnan(x) || isnan(y))
		return x + y;
	if (isinf(y)) {
		if (__builtin_fabs(x) == 1.0)
			return 1.0;
		return (__builtin_fabs(x) < 1.0) == (y < 0) ? INFINITY : 0.0;
	}
	kind = __libc_fast_pow_integer_kind(iy);
	sign = signbit(x) && kind == 1 ? -1.0 : 1.0;
	if (x == 0)
		return y < 0 ? __libc_math_divbyzero(sign) : sign * 0.0;
	if (isinf(x))
		return y < 0 ? sign * 0.0 : sign * INFINITY;
	if (x < 0 && kind == 0)
		return __libc_math_invalid();
	/* From 2^63 y log|x| is beyond the ends for every x other than +-1 (|log x| is at least 2^-53) */
	if (__builtin_fabs(y) >= 0x1p63) {
		if (__builtin_fabs(x) == 1.0)
			return 1.0;
		return (__builtin_fabs(x) < 1.0) == (y > 0) ? __libc_math_underflow(sign) : __libc_math_overflow(sign);
	}

	/*
	 * Below 2^-65 (not an integer, so x > 0) e^(y log x) is 1 nudged toward y log x, at run time so that it rounds in the
	 * caller's direction (the fast files drop -frounding-math, which would let the sum fold to 1)
	 */
	volatile double nudge = 0x1p-60;

	return 1.0 + ((y > 0) == (x > 1.0) ? nudge : -nudge);
}
