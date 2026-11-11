/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"
#include "libm-fixed.h"

/*
 * The fast log2f of the cores without an FPU (PLAN D162, D172): nofp/speed's fixed-point path for every positive
 * finite x (the fixed-point log2 core) rounded to nearest without the rounding
 * test, below 0.5 + 8/512 ulp, and n exactly at 2^n; the float-float path (PLAN D58) for the special values only (the
 * note, section 15).
 */
float log2f(float x)
{
	uint32_t bits = __libc_float_bits(x);

	if (bits - 1 < 0x7f7fffff) {
		bool negative;
		int exponent;
		uint64_t v = __libc_log2f_fixed(bits, &exponent, &negative);

		if (v == 0)
			return 0.0f;
		return __libc_float_from(__libc_fixed_nearest((uint32_t)(v >> 31), exponent) | (negative ? 0x80000000u : 0));
	}

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log(+-0) is -infinity with divide-by-zero */
	if (x == 0)
		return -1.0f / fabsf(x);
	/* +infinity: what the fixed-point path leaves */
	return x;
}
