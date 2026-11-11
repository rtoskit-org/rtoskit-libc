/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10p1f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast log10p1f of the cores without an FPU (PLAN D162, D174): below 2^-5 the fixed-point series of log(1 + x) / ln 10 on x's
 * significand (__libc_fixed_series); from 2^-5 up, 1 + x exactly (__libc_fixed_one_plus) through the log2 core of
 * logf, times log10 2 to 64 bits, rounded to nearest without a test; below 1 ulp to nearest (the note, section 17). Special values as C23
 * F.10.3.8.
 */
float log10p1f(float x)
{
	/* (-1/2 + x/3 - ... + x^5/7) / ln 10 in Q31 and 1/ln 10 the leading term: log10(1 + x) = x (1 + x p(x)) / ln 10 */
	static const int32_t coefficients[6] = { (int32_t)0xe43484eb, 0x1287a763, (int32_t)0xf21a4275, 0x0b1e313c, (int32_t)0xf6bc2c4e, 0x07f0fe98 };
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;

	if (magnitude < 0x3d000000) {
		/*
		 * Below 2^-60, x 1/ln 10 to far below an ulp: x's significand (subnormals normalised) times the leading constant,
		 * rounded once, subnormal results included; +-0 keeps its sign
		 */
		if (magnitude < 0x21800000) {
			if (magnitude == 0)
				return x;
			return __libc_fixed_tiny(magnitude, negative, 0x3796f62au, 0, -62);
		}
		return __libc_fixed_series(((bits & __LIBC_FLOAT_FRACTION) | 0x00800000) << 8, (int)(magnitude >> 23) - 127, negative, coefficients, 0x3796f62au);
	}
	if (magnitude < 0x7f800000 && (!negative || magnitude < 0x3f800000)) {
		int e;
		/* The core returns early, with exponent unset, only for 1 + x = 1, which x below 2^-5 never reaches */
		int exponent = 0;
		uint32_t m = __libc_fixed_one_plus(bits, &e);
		uint64_t v = __libc_log2_fixed_core(m, e, &exponent, &negative);
		uint32_t high = (uint32_t)(v >> 32);
		/* v log10 2 2^-65: the product's top 64 bits, in [2^62, 2^64), normalised to bit 63 (log10f's) */
		uint64_t product = __libc_fixed_wide(high, 0x9a209a84) + (__libc_fixed_wide(high, 0xfbcff799) >> 32) + (__libc_fixed_wide((uint32_t)v, 0x9a209a84) >> 32);

		--exponent;
		if ((product >> 63) == 0) {
			product <<= 1;
			--exponent;
		}
		return __libc_float_from(__libc_fixed_nearest((uint32_t)(product >> 31), exponent) | (negative ? 0x80000000u : 0));
	}

	if (isnan(x))
		return x + x;
	/* log(1 + -1) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == -1.0f)
		return -1.0f / fabsf(x + 1.0f);
	if (x < -1.0f)
		return __libc_math_invalidf();
	/* +infinity */
	return x;
}
