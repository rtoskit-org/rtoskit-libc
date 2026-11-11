/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10m1f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed-m1.h"

/*
 * The fast exp10m1f of the cores without an FPU (PLAN D162, D173): below 0.054, the fixed-point series on u = x ln 10
 * (the significand times ln 10 to 64 bits, 0x935d8dddaaa8ac17 2^-62, kept to 32); for -8 <= x < 37.5, z = x log2 10 in
 * units of 2^-37 as exp10f's, then 10^x - 1 from the exp2 core (__libc_exp2m1f_fixed); rounded once, below 1 ulp to
 * nearest (the note, section 16). Special values as C23 F.10.3.3.
 */
float exp10m1f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
	int exponent = __libc_float_exponent(bits);

	if (magnitude < 0x3d5e0000) {
		uint64_t product;

		/* Below 2^-62, x ln 10 rounded once, +-0 and the subnormal results included */
		if (magnitude < 0x20800000)
			return x * 0x1.26bb1cp+1f;
		/* u = significand ln 10 2^-32 2^(exponent - 180), the product in [2^54, 2^56) */
		product = __libc_fixed_wide(significand, 0x935d8ddd) + (__libc_fixed_wide(significand, 0xaaa8ac17) >> 32);
		if ((product >> 55) != 0)
			return __libc_expm1f_series_fixed((uint32_t)(product >> 24), exponent - 125, negative);
		return __libc_expm1f_series_fixed((uint32_t)(product >> 23), exponent - 126, negative);
	}
	if (magnitude < 0x42160000 && !(negative && magnitude > 0x41000000)) {
		int shift = 143 - exponent;
		/* significand log2 10 2^-32 2^62, below 2^56; the shift is below 22 from 0.054 up */
		int64_t z = (int64_t)((__libc_fixed_wide(significand, 0xd49a784b) + (__libc_fixed_wide(significand, 0xcd1b8afe) >> 32)) >> shift);

		return __libc_exp2m1f_fixed(negative ? -z : z);
	}

	if (isnan(x))
		return x + x;
	/* 10^x is below a quarter of -1's ulp below 1 (-infinity included): -1 */
	if (negative)
		return -1.0f + 0x1p-100f;
	/* Beyond, the 1 is far below the result's ulp: exp10f's result, its overflow and +infinity */
	return exp10f(x);
}
