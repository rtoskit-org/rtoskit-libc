/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanpif.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast atanpif of the cores without an FPU (PLAN D162, D182): atan(x)/pi from the fixed-point kernel
 * (__libc_fixed_atan) times 1/pi, or 1/2 - atan(1/|x|)/pi, rounded to nearest once (subnormal results included). From 2^27
 * 1/2 - 1/(pi x) rounds to 1/2 (C23 F.10.1.3: +-0 itself, +-infinity +-1/2).
 */
float atanpif(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	int exponent;

	if (magnitude == 0)
		return x;
	if (magnitude >= 0x4d000000)
		return magnitude > 0x7f800000 ? x + x : copysignf(0.5f, x);
	if (magnitude <= 0x3f800000) {
		v = __libc_fixed_atan(magnitude, 0x3f800000, &exponent);
		v = __libc_fixed_inverse_pi(v, &exponent);
		return __libc_fixed_product_float(v, exponent - 63, negative);
	}

	v = __libc_fixed_atan(0x3f800000, magnitude, &exponent);
	v = __libc_fixed_inverse_pi(v, &exponent);
	return __libc_fixed_offset_float(UINT64_C(1) << 61, v, exponent, true, negative);
}
