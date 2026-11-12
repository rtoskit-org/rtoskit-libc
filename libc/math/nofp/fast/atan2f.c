/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2f.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast atan2f of the cores without an FPU (PLAN D162, D182): the special cases of C23 F.10.1.4 as the strict file;
 * otherwise theta = atan(min/max) of the magnitudes by the fixed-point kernel (__libc_fixed_atan) and, by the quadrant,
 * theta, pi - theta, pi/2 - theta or pi/2 + theta, rounded to nearest once (subnormal results included).
 */
float atan2f(float y, float x)
{
	uint32_t ybits = __libc_float_bits(y);
	uint32_t xbits = __libc_float_bits(x);
	uint32_t ay = ybits & 0x7fffffff;
	uint32_t ax = xbits & 0x7fffffff;
	bool negative = (ybits >> 31) != 0;
	bool left = (xbits >> 31) != 0;
	uint64_t v;
	int exponent;

	if (ax > 0x7f800000 || ay > 0x7f800000)
		return x + y;
	if (ay == 0)
		return left ? copysignf(0x1.921fb6p1f, y) : y;
	if (ax == 0)
		return copysignf(0x1.921fb6p0f, y);
	if (ay == 0x7f800000)
		return copysignf(ax == 0x7f800000 ? (left ? 0x1.2d97c8p1f : 0x1.921fb6p-1f) : 0x1.921fb6p0f, y);
	if (ax == 0x7f800000)
		return left ? copysignf(0x1.921fb6p1f, y) : copysignf(0.0f, y);

	if (ay <= ax) {
		v = __libc_fixed_atan(ay, ax, &exponent);
		if (!left)
			return __libc_fixed_product_float(v, exponent - 63, negative);
		return __libc_fixed_offset_float(__LIBC_FIXED_PI_62, v, exponent, true, negative);
	}

	v = __libc_fixed_atan(ax, ay, &exponent);
	return __libc_fixed_offset_float(__LIBC_FIXED_HALF_PI_62, v, exponent, !left, negative);
}
