/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"
#include "libm-fixed.h"

/*
 * The fast exp2f of the cores without an FPU (PLAN D162, D172): nofp/speed's fixed-point path for |x| < 126 (z = x 2^37
 * as an integer, the fixed-point exp2 core) rounded to nearest without the rounding test, below 0.5 + 6/512 ulp, and
 * exact at the integers (f is 0); the float-float path (PLAN D57) for the other arguments only (the note, section 15).
 */
float exp2f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	int exponent;
	struct __libc_ff value;

	if ((bits & 0x7fffffff) < 0x42fc0000) {
		int shift = __libc_float_exponent(bits) - 113;
		uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
		int64_t z = 0;
		uint32_t f;

		if (shift >= 0)
			z = (int64_t)significand << shift;
		else if (shift > -24)
			z = significand >> -shift;
		if ((bits >> 31) != 0)
			z = -z;

		f = __libc_exp2f_fixed(z, &exponent);
		return __libc_float_from(__libc_fixed_nearest(f, exponent));
	}

	if (isnan(x))
		return x + x;
	if (x >= 128.0f)
		return x * 0x1p127f;
	if (x < -151.0f)
		return 0x1p-100f * 0x1p-100f;

	value = __libc_exp2f_core((struct __libc_ff){ .hi = x, .lo = 0.0f }, &exponent);

	return __libc_exp2f_scale(value, exponent);
}
