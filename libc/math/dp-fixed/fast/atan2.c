/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"
#include "dp-fixed-fast.h"

/*
 * The fast atan2 of the soft-double cores (PLAN D162, D199; docs/research/libm-fast-double.md, section 17): for normal,
 * finite, nonzero coordinates whose exponents differ by at most 60, the fixed-point kernel on the smaller magnitude over
 * the larger, with the quadrant's offset and y's sign, one rounding; every other case (C23 7.12.4.4, F.10.1.4's zeros,
 * infinities and NaN, subnormals and the far exponents) is the strict file's code (D198's slow path).
 */
double atan2(double y, double x)
{
	uint64_t by = __libc_double_bits(y);
	uint64_t bx = __libc_double_bits(x);
	uint64_t ay = by & 0x7fffffffffffffffull;
	uint64_t ax = bx & 0x7fffffffffffffffull;
	uint32_t ey = (uint32_t)(ay >> 52);
	uint32_t ex = (uint32_t)(ax >> 52);
	uint64_t my = (ay & 0x000fffffffffffffull) | 0x0010000000000000ull;
	uint64_t mx = (ax & 0x000fffffffffffffull) | 0x0010000000000000ull;
	bool west = (bx >> 63) != 0;
	bool negative = (by >> 63) != 0;

	/* Both biased exponents in [1, 2046] and at most 60 apart, as unsigned differences */
	if (ey - 1 >= 2046 || ex - 1 >= 2046 || ey - ex + 60 > 120)
		return __libc_fast_atan2_slow(y, x);

	if (ay <= ax)
		return __libc_dp_fixed_atan(my, (int32_t)ey, mx, (int32_t)ex, west ? 3 : 0, negative);

	return __libc_dp_fixed_atan(mx, (int32_t)ex, my, (int32_t)ey, west ? 2 : 1, negative);
}
