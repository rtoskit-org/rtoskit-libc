/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinhf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"
#include "internal/math.h"

/* (asinh(x)/x - 1)/u for u = x^2 <= 1/4 by its Chebyshev fit of degree 6 in signed Q31 (u p(u) within 2^-34.7) */
static const int32_t series[7] = { -357913941, 161061237, -95867432, 65186503, -47352405, 32915512, -15376772 };

/*
 * The fast asinhf of the cores without an FPU (PLAN D162, D185), in fixed point and rounded to nearest once: below 1/2 the odd
 * series x K(x^2); above, log(|x| + sqrt(x^2 + 1)) with x^2 + 1 exact in 64 bits, the fixed-point square root and the log2
 * core (__libc_fixed_log_root), log(2|x|) from 2^12 (C23 F.10.2.2: +-0 and +-infinity themselves). Below 2^-12 the x^3/6
 * term is below half an ulp.
 */
float asinhf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t v;
	int exponent;

	if (magnitude < 0x39800000)
		return x;
	if (magnitude >= 0x7f800000)
		return x + x;
	if (magnitude <= 0x3f000000)
		v = __libc_fixed_odd_series(magnitude, series, 6, &exponent);
	else
		v = __libc_fixed_log_root(magnitude, false, &exponent);
	return __libc_fixed_float(v, exponent, negative);
}
