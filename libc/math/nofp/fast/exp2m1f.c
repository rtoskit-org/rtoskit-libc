/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2m1f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast exp2m1f of the cores without an FPU (PLAN D162, D173): below 0.18, the fixed-point series on u = x ln 2 (the
 * significand times ln 2 to 64 bits, 0xb17217f7d1cf79ac 2^-64, kept to 32), and u itself below 2^-59; for -26 <= x < 126, z = x 2^37 as exp2f's,
 * then 2^x - 1 from the exp2 core (__libc_exp2m1f_fixed); rounded once, below 1 ulp to nearest (the note, section 16).
 * Special values as C23 F.10.3.5.
 */
float exp2m1f(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
	int exponent = __libc_float_exponent(bits);

	if (magnitude < 0x3e380000) {
		uint64_t product;

		/*
		 * Below 2^-59, e^u - 1 is u = x ln 2 to far below an ulp: x's significand (subnormals normalised) times
		 * ln 2 to 64 bits, rounded once, subnormal results included; +-0 keeps its sign
		 */
		if (magnitude < 0x22000000) {
			if (magnitude == 0)
				return x;
			return __libc_fixed_tiny(magnitude, negative, 0xb17217f7, 0xd1cf79ac, -63);
		}
		/* u = significand ln 2 2^-32 2^(exponent - 182), the product in [2^54, 2^56) */
		product = __libc_fixed_wide(significand, 0xb17217f7) + (__libc_fixed_wide(significand, 0xd1cf79ac) >> 32);
		if ((product >> 55) != 0)
			return __libc_expm1f_series_fixed((uint32_t)(product >> 24), exponent - 127, negative);
		return __libc_expm1f_series_fixed((uint32_t)(product >> 23), exponent - 128, negative);
	}
	if (magnitude < 0x42fc0000 && !(negative && magnitude > 0x41d00000)) {
		int shift = exponent - 113;
		/* x 2^37, exact: the shift is at least 9 from 0.18 up */
		int64_t z = (int64_t)significand << shift;

		return __libc_exp2m1f_fixed(negative ? -z : z);
	}

	if (isnan(x))
		return x + x;
	/* 2^x is below a quarter of -1's ulp below 1 (-infinity included): -1 */
	if (negative)
		return -1.0f + 0x1p-100f;
	/* Beyond, the 1 is far below the result's ulp: exp2f's result, its overflow and +infinity */
	return exp2f(x);
}
