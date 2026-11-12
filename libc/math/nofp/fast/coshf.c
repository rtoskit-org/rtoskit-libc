/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * coshf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast coshf of the cores without an FPU (PLAN D162, D184), in fixed point and rounded to nearest once: up to 11 e^|x|/2
 * + e^-|x|/2 by the exp2 core twice; beyond, e^|x|/2 alone (e^-2|x| is below 2^-31), and above 0x1.65a9f8p+6 the overflow
 * (C23 F.10.2.4: 1 at +-0, +infinity at +-infinity). Below 2^-12 the x^2/2 term is below half an ulp of 1.
 */
float coshf(float x)
{
	uint32_t magnitude = __libc_float_bits(x) & 0x7fffffff;
	uint32_t f;
	int exponent;

	if (magnitude < 0x39800000)
		return 1.0f;
	if (magnitude < 0x41300000)
		return __libc_fixed_cosh_sinh(__libc_fixed_log2e(magnitude), false, false);
	if (magnitude <= 0x42b2d4fc) {
		f = __libc_exp2f_fixed(__libc_fixed_log2e(magnitude) - (INT64_C(1) << 37), &exponent);
		return __libc_float_from(__libc_fixed_nearest(f, exponent));
	}
	return magnitude > 0x7f800000 ? x + x : fabsf(x) * 0x1p127f;
}
