/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanhf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/* (tanh(x)/x - 1)/u for u = x^2 <= 1/4 by its Chebyshev fit of degree 5 in signed Q31 (u p(u) within 2^-33.6) */
static const int32_t series[6] = { -715827882, 286331073, -115892174, 46899068, -18515220, 5754703 };

/*
 * The fast tanhf of the cores without an FPU (PLAN D162, D184), in fixed point and rounded to nearest once: below 1/2 the odd
 * series x K(x^2); up to 9.1 1 - 2/(e^2|x| + 1), e^2|x| = 2^n (1 + f 2^-32) by the exp2 core, D = e^2|x| + 1 exact in 64
 * bits, 2/D by the fixed-point quotient (the result is at least 0.46, so an absolute 2^-30 is 1/32 of its ulp); beyond, +-1
 * (2 e^-2|x| is below a quarter ulp of 1). C23 F.10.2.6: +-0 itself, +-1 at +-infinity. Below 2^-12, x.
 */
float tanhf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint64_t d;
	uint64_t q;
	uint64_t r;
	uint32_t f;
	uint32_t top;
	int exponent;
	int k;

	if (magnitude < 0x39800000)
		return x;
	if (magnitude <= 0x3f000000) {
		d = __libc_fixed_odd_series(magnitude, series, 5, &exponent);
		return __libc_fixed_float(d, exponent, negative);
	}
	if (magnitude >= 0x4111999a)
		return magnitude > 0x7f800000 ? x + x : copysignf(1.0f, x);

	/* e^2|x| + 1 in units of 2^-32 (n in 1 .. 27, below 2^61), its top 32 bits, and 2/D = q 2^(k - 62) */
	f = __libc_exp2f_fixed(2 * __libc_fixed_log2e(magnitude), &exponent);
	d = (((UINT64_C(1) << 32) + f) << exponent) + (UINT64_C(1) << 32);
	k = (int)stdc_leading_zeros(d);
	top = (uint32_t)((d << k) >> 32);
	q = top == 0x80000000u ? UINT64_C(1) << 32 : __libc_fixed_quotient(0x80000000u, top);

	/* 1 - 2/D in units of 2^-62, at least 0.46 */
	r = (UINT64_C(1) << 62) - (q << k);
	k = (int)stdc_leading_zeros(r);
	return __libc_fixed_float(r << k, 1 - k, negative);
}
