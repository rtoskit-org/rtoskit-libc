/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acosf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"
#include "internal/math.h"

/*
 * The fast acosf of the cores without an FPU (PLAN D162, D183): from the fixed-point kernel (__libc_fixed_asin), pi/2 -
 * asin x up to 1/2, 2 asin sqrt((1 - x)/2) above and pi minus it below -1/2, rounded to nearest once (C23 F.10.1.1: +0 at
 * 1, |x| > 1 invalid).
 */
float acosf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	int exponent;
	bool outer;

	if (magnitude < 0x32800000)
		return 0x1.921fb6p0f;
	if (magnitude > 0x3f800000)
		return magnitude > 0x7f800000 ? x + x : __libc_math_invalidf();
	if (bits == 0x3f800000)
		return 0.0f;

	v = __libc_fixed_asin(magnitude, &exponent, &outer);
	if (!outer)
		return __libc_fixed_offset_float(__LIBC_FIXED_HALF_PI_62, v, exponent, !negative, false);
	if (!negative)
		return __libc_fixed_float(v, exponent, false);
	return __libc_fixed_offset_float(__LIBC_FIXED_PI_62, v, exponent, true, false);
}
