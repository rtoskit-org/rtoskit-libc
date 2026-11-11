/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erff.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast erff of the single-precision FPU cores (PLAN D162, D178): below 1/2 (2/sqrt(pi)) x + x^3 Q(x^2) with the
 * constant as a pair and its product's error by a fused multiply-add; above, 1 - erfc|x| from the pair kernel
 * (__libc_fast_erfcf_kernel), its pair accurate enough that the subtraction from 1 costs nothing; up to 0.875 the series'
 * float tail measured 1.23 ulp. Below 1.5 ulp to nearest (the trigonometric note, section 13);
 * float only, round to nearest only as the layer (D57). C23 F.10.5.1: +-0 itself, +-infinity +-1.
 */
float erff(float x)
{
	float a = __builtin_fabsf(x);
	float t;
	float z;
	float p;
	float hi;
	float lo;
	float s;
	float error;
	int k;

	/* NaN, and +-0 with its sign: the scaled product below would give -0 (+0 for -0 + +0) */
	if (isnan(x) || x == 0)
		return x + x;
	/* erfc(3.92) is below a quarter ulp of 1: +-1, as the other build types; the infinities included */
	if (a >= 3.92f)
		return x < 0 ? -1.0f : 1.0f;
	if (a < 0.5f) {
		/*
		 * Below 2^-13 the x^3 term is under 2^-27.6 of the result: (2/sqrt(pi)) x with x scaled by 2^64, so that neither
		 * the low part's product nor a subnormal result rounds twice, and one rounding back (+-0 kept)
		 */
		if (a < 0x1p-13f) {
			t = x * 0x1p64f;
			return __builtin_fmaf(t, 0x1.20dd76p+0f, t * -0x1.f9a0e6p-25f) * 0x1p-64f;
		}
		z = x * x;
		p = 0x1.20dd76p+0f * x;
		return p + (__builtin_fmaf(0x1.20dd76p+0f, x, -p) + __builtin_fmaf(-0x1.f9a0e6p-25f, x, x * z * __libc_fast_erff_tail(z)));
	}
	hi = __libc_fast_erfcf_kernel(a, &k, &lo);
	/* k >= -23 here: 2^k scales both parts exactly */
	s = __libc_float_from((uint32_t)(k + 127) << 23);
	hi = __libc_fast_two_sum(1.0f, -(hi * s), &error);
	hi = hi + (error - lo * s);

	return x < 0 ? -hi : hi;
}
