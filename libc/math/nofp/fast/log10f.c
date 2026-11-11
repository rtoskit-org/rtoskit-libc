/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10f.c
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
 * The fast log10f of the cores without an FPU (PLAN D162, D172): nofp/speed's fixed-point path for every positive
 * finite x (the fixed-point log2 core, times log10 2 to 64 bits) rounded to nearest without the rounding
 * test, below 0.5 + 8/512 ulp; the float-float path (PLAN D58) for the special values only (the
 * note, section 15).
 */
float log10f(float x)
{
	uint32_t bits = __libc_float_bits(x);

	if (bits - 1 < 0x7f7fffff) {
		bool negative;
		int exponent;
		uint64_t v = __libc_log2f_fixed(bits, &exponent, &negative);
		uint32_t high = (uint32_t)(v >> 32);
		uint64_t product;

		if (v == 0)
			return 0.0f;

		/* v log10 2 2^-65: the product's top 64 bits, in [2^62, 2^64), normalised to bit 63 */
		product = __libc_fixed_wide(high, 0x9a209a84) + (__libc_fixed_wide(high, 0xfbcff799) >> 32) + (__libc_fixed_wide((uint32_t)v, 0x9a209a84) >> 32);
		--exponent;
		if ((product >> 63) == 0) {
			product <<= 1;
			--exponent;
		}
		return __libc_float_from(__libc_fixed_nearest((uint32_t)(product >> 31), exponent) | (negative ? 0x80000000u : 0));
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
