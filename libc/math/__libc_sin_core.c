/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_sin_core.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * sin(r) for |r| <= 0.8: r - r^3 (1/6 - r^2/120) with the cube and the
 * bracket as double-doubles, so the part left to plain double arithmetic,
 * r^7 q(r^2), stays below 2^-14 and its roundings below 2^-66. The bracket
 * takes 1/120 as 1/128 + 1/1920: r^2/128 is exact and goes into the two-sum,
 * and only r^2/1920 (at most 2^-11.5) is rounded. q is a near-minimax fit of
 * five terms (2^-67 relative on |r| <= 0.8). The low part of r enters through
 * cos(r) ~ 1 - r^2/2 + r^4/24: dropping r^4/24 would cost 2^-60, a hundredth
 * of an ulp.
 */
struct __libc_dd __libc_sin_core(struct __libc_dd r)
{
	const struct __libc_dd sixth = { .hi = 0x1.5555555555555p-3, .lo = 0x1.5555555555555p-57 };
	double x = r.hi;
	double z = x * x;
	double z2 = z * z;
	struct __libc_dd square = __libc_dd_product(x, x);
	struct __libc_dd cube = __libc_dd_multiply(square, (struct __libc_dd){ .hi = x, .lo = 0.0 });
	struct __libc_dd bracket = __libc_dd_fast_sum(sixth.hi, -0x1p-7 * square.hi);
	struct __libc_dd term = __libc_dd_multiply(cube, (struct __libc_dd){ .hi = bracket.hi, .lo = bracket.lo + (sixth.lo - 0x1p-7 * square.lo - z * 0x1.1111111111111p-11) });
	double tail = x * z * z2 * (-0x1.a01a01a019b60p-13 + z * (0x1.71de3a546ec13p-19 + z * (-0x1.ae6453fda193dp-26 + z * (0x1.61215b2e295d9p-33 + z * -0x1.aafc87d0f6ea0p-41))));
	struct __libc_dd sum = __libc_dd_fast_sum(x, -term.hi);

	return __libc_dd_fast_sum(sum.hi, sum.lo - term.lo + tail + r.lo * (1.0 - 0.5 * z + z2 * 0x1.5555555555555p-5));
}
