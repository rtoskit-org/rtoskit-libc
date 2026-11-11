/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10f.c
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
 * The fast exp10f of the cores without an FPU (PLAN D162, D172): for |x| < 37.5 (a normal result, |x log2 10| < 125),
 * z = x log2 10 in units of 2^-37 from the 24-bit significand times log2 10 to 64 bits (0xd49a784bcd1b8afe 2^-62), in
 * error by less than 2^-35 as expf's, through the fixed-point exp2 core, rounded to nearest without a rounding test:
 * below 0.5 + 6/512 ulp. The float-float path (PLAN D57) takes the other arguments (the note, section 15).
 */
float exp10f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	int exponent;
	struct __libc_ff value;

	if ((bits & 0x7fffffff) < 0x42160000) {
		int shift = 143 - __libc_float_exponent(bits);
		uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
		int64_t z = 0;
		uint32_t f;

		/* significand log2 10 2^-32 2^62, below 2^56 */
		if (shift < 56)
			z = (int64_t)((__libc_fixed_wide(significand, 0xd49a784b) + (__libc_fixed_wide(significand, 0xcd1b8afe) >> 32)) >> shift);
		if ((bits >> 31) != 0)
			z = -z;

		f = __libc_exp2f_fixed(z, &exponent);
		return __libc_float_from(__libc_fixed_nearest(f, exponent));
	}

	if (isnan(x))
		return x + x;
	if (x > 39.0f)
		return x * 0x1p127f;
	if (x < -46.0f)
		return 0x1p-100f * 0x1p-100f;

	value = __libc_exp2f_core(__libc_ff_scale(x, __LIBC_LOG2_10F), &exponent);

	return __libc_exp2f_scale(value, exponent);
}
