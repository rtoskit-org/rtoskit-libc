/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinhf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/* (sinh(x)/x - 1)/u for u = x^2 <= 1/4 by its Chebyshev fit of degree 3 in signed Q31 (u p(u) within 2^-34.4) */
static const int32_t series[4] = { 357913941, 17895697, 426084, 5945 };

/*
 * The fast sinhf of the cores without an FPU (PLAN D162, D184), in fixed point and rounded to nearest once: below 1/2 the odd
 * series x K(x^2); up to 11 e^|x|/2 - e^-|x|/2 by the exp2 core twice (the difference is at least 0.52); beyond, e^|x|/2
 * alone (e^-2|x| is below 2^-31), and above 0x1.65a9f8p+6 the overflow (C23 F.10.2.5: +-0 itself, +-infinity itself).
 * Below 2^-12 the x^3/6 term is below half an ulp.
 */
float sinhf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	uint32_t f;
	int exponent;

	if (magnitude < 0x39800000)
		return x;
	if (magnitude <= 0x3f000000) {
		v = __libc_fixed_odd_series(magnitude, series, 3, &exponent);
		return __libc_fixed_float(v, exponent, negative);
	}
	if (magnitude < 0x41300000)
		return __libc_fixed_cosh_sinh(__libc_fixed_log2e(magnitude), true, negative);
	if (magnitude <= 0x42b2d4fc) {
		f = __libc_exp2f_fixed(__libc_fixed_log2e(magnitude) - (INT64_C(1) << 37), &exponent);
		return __libc_float_from(__libc_fixed_nearest(f, exponent) | (bits & 0x80000000u));
	}
	return magnitude > 0x7f800000 ? x + x : x * 0x1p127f;
}
