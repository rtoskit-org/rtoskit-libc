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

/* +-k pi/4 with y's sign, from the pair, rounded once */
static double signed_pi(double y, double quarters)
{
	return copysign(__LIBC_PI.hi * quarters, y) + copysign(__LIBC_PI.lo * quarters, y);
}

/*
 * The fast atan2's slow path (PLAN D198, D211; docs/research/libm-fast-double.md section 27), for the double-FPU and
 * the soft-double cores' fast atan2 alike, without the strict core (P11.06g). C23 7.12.4.4, F.10.1.4: the zeros,
 * infinities and NaN first. Then, for finite nonzero coordinates:
 * - exponents more than 60 apart: +-pi/2 when y is the larger (pi/2 - x/y rounds to pi/2), +-pi when x < 0 is, and
 *   for x > 0 atan(y/x) = q - q^3/3 with |q| < 2^-59: the quotient rounded once, or when it is exact (q x == y as a
 *   pair) nudged toward 0 by __libc_math_tiny, so that inexact and underflow are raised as for the function;
 * - otherwise one of them is subnormal, below 2^-959 or from 2^960: both scaled by the same power of two (exact: the
 *   smaller stays normal within 61 binades of 1), which both fast paths then take, through atan2 itself.
 */
double __libc_fast_atan2_slow(double y, double x)
{
	struct __libc_dd product;
	int difference;
	int scale;
	double q;

	if (isnan(x) || isnan(y))
		return x + y;
	if (y == 0)
		return signbit(x) ? signed_pi(y, 1.0) : y;
	if (x == 0)
		return signed_pi(y, 0.5);
	if (isinf(y))
		return signed_pi(y, isinf(x) ? (x > 0 ? 0.25 : 0.75) : 0.5);
	if (isinf(x))
		return x > 0 ? copysign(0.0, y) : signed_pi(y, 1.0);
	difference = ilogb(y) - ilogb(x);
	if (difference > 60)
		return signed_pi(y, 0.5);
	if (difference < -60) {
		if (x < 0)
			return signed_pi(y, 1.0);
		q = y / x;
		product = __libc_dd_product(q, x);
		/* An exact quotient: atan(q) lies just inside it, nudged with inexact (and underflow when subnormal) */
		if (product.hi == y && product.lo == 0)
			return __libc_math_tiny(q, -q);
		/* Otherwise the division's rounding, flags and zero sign are atan's (q^3/3 is far below half an ulp) */
		return q;
	}
	scale = -ilogb(fabs(x) > fabs(y) ? x : y);
	return atan2(scalbn(y, scale), scalbn(x, scale));
}
