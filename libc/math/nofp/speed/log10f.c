/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10f.c
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"

/*
 * C23 7.12.6.12: log2 x from the fixed-point core times log10 2 to 64 bits
 * (0x9a209a84fbcff799 2^-65), for a positive finite x; the float-float path
 * (PLAN D58) takes the other arguments and the results the fixed-point one
 * cannot round with certainty (PLAN D86, D114).
 */
float log10f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	struct __libc_ff value;

	if (bits - 1 < 0x7f7fffff) {
		bool negative;
		int exponent;
		uint64_t v = __libc_log2f_fixed(bits, &exponent, &negative);
		uint32_t high = (uint32_t)(v >> 32);
		uint64_t product;
		uint32_t result;

		if (v == 0)
			return 0.0f;

		/* v log10 2 2^-65: the product's top 64 bits, in [2^62, 2^64), normalised to bit 63 */
		product = __libc_fixed_wide(high, 0x9a209a84) + (__libc_fixed_wide(high, 0xfbcff799) >> 32) + (__libc_fixed_wide((uint32_t)v, 0x9a209a84) >> 32);
		--exponent;
		if ((product >> 63) == 0) {
			product <<= 1;
			--exponent;
		}
		if (__libc_fixed_round((uint32_t)(product >> 31), exponent, __LIBC_FIXED_LOG2_ERROR, &result))
			return __libc_float_from(result | (negative ? 0x80000000u : 0));
	}

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log(+-0) is -infinity with divide-by-zero */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	value = __libc_log2f_core(x, 0.0f);
	value = __libc_ff_multiply(value, __LIBC_LOG10_2F);

	return value.hi + value.lo;
}
