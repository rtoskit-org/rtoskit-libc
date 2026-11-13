/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_atan2_slow.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast atan2's slow path (PLAN D198): the strict file's code, unchanged, for the zeros, infinities, NaN,
 * subnormal and huge coordinates and the far exponents that the fast path leaves.
 *
 * C23 7.12.4.4, F.10.1.4: the special cases first; then the coordinates are
 * scaled by a power of two (the angle does not change) and the core does the
 * rest. A tiny quotient with x > 0 is y/x, rounded once; a quotient beyond
 * 2^60 (or below 2^-60 with x < 0) leaves +-pi/2 (+-pi) once rounded.
 */
double __libc_fast_atan2_slow(double y, double x)
{
	struct __libc_dd value;
	int scale;
	int mode;

	if (isnan(x) || isnan(y))
		return x + y;
	if (y == 0) {
		if (signbit(x))
			return copysign(__LIBC_PI.hi, y) + copysign(__LIBC_PI.lo, y);
		return y;
	}
	if (x == 0)
		return copysign(__LIBC_PI_2.hi, y) + copysign(__LIBC_PI_2.lo, y);
	if (isinf(y)) {
		if (isinf(x)) {
			double quarter = x > 0 ? 0.25 : 0.75;

			return copysign(__LIBC_PI.hi * quarter, y) + copysign(__LIBC_PI.lo * quarter, y);
		}
		return copysign(__LIBC_PI_2.hi, y) + copysign(__LIBC_PI_2.lo, y);
	}
	if (isinf(x)) {
		if (x > 0)
			return copysign(0.0, y);
		return copysign(__LIBC_PI.hi, y) + copysign(__LIBC_PI.lo, y);
	}
	if (ilogb(y) - ilogb(x) > 60)
		return copysign(__LIBC_PI_2.hi, y) + copysign(__LIBC_PI_2.lo, y);
	if (x < 0 && ilogb(y) - ilogb(x) < -60)
		return copysign(__LIBC_PI.hi, y) + copysign(__LIBC_PI.lo, y);

	scale = -(ilogb(fabs(x) > fabs(y) ? x : y));
	if (x > 0 && ilogb(y) - ilogb(x) < -1080)
		return __libc_math_underflow(copysign(1.0, y));
	if (x > 0 && ilogb(y) - ilogb(x) < -60) {
		/* atan(q) = q - q^3/3: an exact quotient is nudged toward zero, then one rounding */
		mode = __libc_math_nearest();
		value = __libc_dd_divide((struct __libc_dd){ .hi = scalbn(y, scale + 200), .lo = 0.0 }, (struct __libc_dd){ .hi = scalbn(x, scale), .lo = 0.0 });
		if (value.lo == 0)
			value.lo = -value.hi * 0x1p-80;
		__libc_math_restore(mode);
		return __libc_exp_scale(value, -200);
	}

	mode = __libc_math_nearest();
	value = __libc_atan2_core((struct __libc_dd){ .hi = scalbn(y, scale), .lo = 0.0 }, (struct __libc_dd){ .hi = scalbn(x, scale), .lo = 0.0 });
	__libc_math_restore(mode);

	return value.hi + value.lo;
}
