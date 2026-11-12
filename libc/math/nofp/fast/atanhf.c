/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanhf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"
#include "internal/math.h"

/* (atanh(x)/x - 1)/u for u = x^2 <= 1/16 by its Chebyshev fit of degree 4 in signed Q31 (u p(u) within 2^-36.3) */
static const int32_t series[5] = { 715827883, 429496447, 306819321, 237019314, 223489701 };

/*
 * The fast atanhf of the cores without an FPU (PLAN D162, D185), in fixed point and rounded to nearest once: below 1/4 the
 * odd series x K(x^2); above, -log((1 - |x|)/(1 + |x|))/2 with 1 - |x| and 1 + |x| exact integers, their quotient by the
 * fixed-point division (in (0, 0.6]) and its logarithm by the log2 core (the result is at least 0.255, so an absolute 2^-31 is
 * 1/32 of its ulp). C23 F.10.2.3: +-0 itself, +-infinity with divide-by-zero at +-1, |x| > 1 invalid. Below 2^-12 the
 * x^3/3 term is below half an ulp.
 */
float atanhf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint32_t m = (magnitude & __LIBC_FLOAT_FRACTION) | 0x00800000;
	uint32_t one;
	uint32_t n;
	uint32_t d;
	uint64_t v;
	bool sign;
	int exponent;
	int k;

	if (magnitude < 0x39800000)
		return x;
	if (magnitude >= 0x3f800000) {
		if (magnitude == 0x3f800000)
			return x / 0.0f;
		return magnitude > 0x7f800000 ? x + x : __libc_math_invalidf();
	}
	if (magnitude < 0x3e800000) {
		v = __libc_fixed_odd_series(magnitude, series, 4, &exponent);
		return __libc_fixed_float(v, exponent, negative);
	}

	/* |x| = m 2^(e - 23) with e = -2 or -1: 1 -+ |x| = 2^(23 - e) -+ m exactly, below 2^26, normalised to bit 31 */
	one = UINT32_C(1) << (150 - (magnitude >> 23));
	n = one - m;
	d = one + m;
	k = (int)stdc_leading_zeros(d) - (int)stdc_leading_zeros(n);
	n <<= stdc_leading_zeros(n);
	d <<= stdc_leading_zeros(d);
	if (n >= d) {
		n >>= 1;
		++k;
	}

	/* q = (n/d) 2^k in Q32 times 2^(k - 32), as v 2^(exponent - 63) with exponent = k - 1; atanh = -log(q)/2 */
	v = __libc_fixed_log((uint64_t)__libc_fixed_quotient(n, d) << 32, k - 1, &exponent, &sign);
	return __libc_fixed_float(v, exponent - 1, negative);
}
