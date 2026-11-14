/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast atanf of the cores without an FPU (PLAN D162, D182): atan(|x|/1) or pi/2 - atan(1/|x|) by the fixed-point kernel
 * (__libc_fixed_atan), rounded to nearest once. Below 2^-12 the x^3/3 term is below half an ulp, from 2^26 pi/2 - 1/x
 * rounds to pi/2 (C23 F.10.1.3: +-0 itself, +-infinity +-pi/2).
 */
float atanf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	int exponent;

	if (magnitude < 0x39800000)
		return x;
	if (magnitude >= 0x4c800000)
		return magnitude > 0x7f800000 ? x + x : copysignf(0x1.921fb6p0f, x);
	if (magnitude <= 0x3f800000) {
		v = __libc_fixed_atan(magnitude, 0x3f800000, &exponent);
		return __libc_fixed_float(v, exponent, negative);
	}

	v = __libc_fixed_atan(0x3f800000, magnitude, &exponent);
	return __libc_fixed_offset_float(__LIBC_FIXED_HALF_PI_62, v, exponent, true, negative);
}
