/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expm1f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed-m1.h"

/*
 * The fast expm1f of the cores without an FPU (PLAN D162, D173): below 1/8 the fixed-point series on x itself (its
 * significand exact); for -18 <= x < 87, z = x / ln 2 in units of 2^-37 as expf's, then 2^t - 1 from the exp2 core
 * (__libc_exp2m1f_fixed); rounded once, below 1 ulp to nearest (the note, section 16). Special values as C23 F.10.3.6.
 */
float expm1f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;

	if (magnitude < 0x3e000000) {
		/* Below 2^-60 e^x - 1 rounds to x, +-0 and the subnormals included */
		if (magnitude < 0x21800000)
			return x;
		return __libc_expm1f_series_fixed(((bits & __LIBC_FLOAT_FRACTION) | 0x00800000) << 8, (int)(magnitude >> 23) - 127, negative);
	}
	if (magnitude < 0x42ae0000 && !(negative && magnitude > 0x41900000)) {
		int shift = 143 - __libc_float_exponent(bits);
		uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
		/* significand / ln 2 2^-32 2^62, below 2^56; the shift is below 20 from 1/8 up */
		int64_t z = (int64_t)((__libc_fixed_wide(significand, 0x5c551d94) + (__libc_fixed_wide(significand, 0xae0bf85e) >> 32)) >> shift);

		return __libc_exp2m1f_fixed(negative ? -z : z);
	}

	if (isnan(x))
		return x + x;
	/* e^x is below a quarter of -1's ulp below 1 (-infinity included): -1 */
	if (negative)
		return -1.0f + 0x1p-100f;
	/* Beyond, the 1 is far below the result's ulp: expf's result, its overflow and +infinity */
	return expf(x);
}
