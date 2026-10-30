/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pow.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/* 0 for a non-integer, 1 for an odd integer, 2 for an even one */
static int integer_kind(double y)
{
	if (fabs(y) >= 0x1p53)
		return 2;
	if (y != trunc(y))
		return 0;

	return fmod(y, 2.0) != 0 ? 1 : 2;
}

/*
 * C23 7.12.7.5, F.10.4.5: the special cases first, then exp(y log|x|) with
 * the product as a double-double; a power of two to a power giving an integer
 * exponent is exact, as are y = 1 and the special cases; the sign is applied before the last
 * rounding so a directed mode rounds the negative result correctly.
 */
double pow(double x, double y)
{
	int kind;
	bool negative;
	struct __libc_dd log;
	struct __libc_dd z;
	struct __libc_dd value;
	int exponent;

	if (y == 0 || x == 1.0)
		return 1.0;
	if (isnan(x) || isnan(y))
		return x + y;

	kind = integer_kind(y);
	negative = signbit(x) && kind == 1;
	if (isinf(y)) {
		if (fabs(x) == 1.0)
			return 1.0;
		return (fabs(x) < 1.0) == (y < 0) ? INFINITY : 0.0;
	}
	if (x == 0) {
		if (y < 0)
			return __libc_math_divbyzero(negative ? -1.0 : 1.0);
		return negative ? -0.0 : 0.0;
	}
	if (isinf(x)) {
		double result = y < 0 ? 0.0 : INFINITY;

		return negative ? -result : result;
	}
	if (x < 0 && kind == 0)
		return __libc_math_invalid();
	if (y == 1.0)
		return x;
	if (fabs(y) <= 0x1p20 && scalbn(fabs(x), -ilogb(x)) == 1.0) {
		double power = y * ilogb(x);

		if (power == trunc(power) && fma(y, ilogb(x), -power) == 0)
			return scalbn(negative ? -1.0 : 1.0, (int)power);
	}

	log = __libc_log_core(fabs(x));
	if (y * log.hi > 710)
		return __libc_math_overflow(negative ? -1.0 : 1.0);
	if (y * log.hi < -746)
		return __libc_math_underflow(negative ? -1.0 : 1.0);

	z = __libc_dd_product(y, log.hi);
	z = __libc_dd_sum(z.hi, z.lo + y * log.lo);
	/*
	 * exp(z) = 1 + z with |z| below 2^-60. z may have underflowed, even to
	 * zero, and rounded in the caller's direction: its sign, from y and
	 * log|x|, decides the directed roundings (PLAN D125)
	 */
	if (fabs(z.hi) < 0x1p-60)
		return 1.0 + ((y > 0) == (fabs(x) > 1.0) ? 0x1p-60 : -0x1p-60);
	value = __libc_exp_core(z.hi, z.lo, &exponent);
	return __libc_exp_scale(negative ? __libc_dd_negate(value) : value, exponent);
}
