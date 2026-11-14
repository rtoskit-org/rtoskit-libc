/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinpif.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"
#include "internal/math.h"

/*
 * The fast asinpif of the cores without an FPU (PLAN D162, D183): the fixed-point kernel (__libc_fixed_asin) times 1/pi, or
 * 1/2 minus it above 1/2, rounded to nearest once (subnormal results included; C23 7.12.4.9: +-0 itself, +-1/2 at +-1, |x| >
 * 1 invalid).
 */
float asinpif(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	int exponent;
	bool outer;

	if (magnitude == 0)
		return x;
	if (magnitude > 0x3f800000)
		return magnitude > 0x7f800000 ? x + x : __libc_math_invalidf();

	v = __libc_fixed_asin(magnitude, &exponent, &outer);
	if (v != 0)
		v = __libc_fixed_inverse_pi(v, &exponent);
	if (!outer)
		return __libc_fixed_product_float(v, exponent - 63, negative);
	return __libc_fixed_offset_float(UINT64_C(1) << 61, v, exponent, true, negative);
}
