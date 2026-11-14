/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"

/*
 * The fast expf of the cores without an FPU (PLAN D162, D172): nofp/speed's fixed-point path for |x| < 87 (z = x / ln 2
 * in units of 2^-37, the fixed-point exp2 core) rounded to nearest without the rounding test, below
 * 0.5 + 6/512 ulp; the float-float path (PLAN D57) for the other arguments only (the note, section 15).
 */
float expf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	int exponent;
	struct __libc_ff value;

	if ((bits & 0x7fffffff) < 0x42ae0000) {
		int shift = 143 - __libc_float_exponent(bits);
		uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
		int64_t z = 0;
		uint32_t f;

		/* significand / ln 2 2^-32 2^62, below 2^56 */
		if (shift < 56)
			z = (int64_t)((__libc_fixed_wide(significand, 0x5c551d94) + (__libc_fixed_wide(significand, 0xae0bf85e) >> 32)) >> shift);
		if ((bits >> 31) != 0)
			z = -z;

		f = __libc_exp2f_fixed(z, &exponent);
		return __libc_float_from(__libc_fixed_nearest(f, exponent));
	}

	if (isnan(x))
		return x + x;
	if (x > 89.0f)
		return x * 0x1p127f;
	if (x < -104.0f)
		return 0x1p-100f * 0x1p-100f;
	/* e^x rounds to 1 + x */
	if (fabsf(x) < 0x1p-25f)
		return 1.0f + x;

	value = __libc_exp2f_core(__libc_ff_scale(x, __LIBC_INV_LN2F), &exponent);

	return __libc_exp2f_scale(value, exponent);
}
