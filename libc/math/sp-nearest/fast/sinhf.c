/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast sinhf of the single-precision FPU cores (PLAN D162, D177): below 1 the odd Taylor series x + x^3 S(x^2) to x^11;
 * up to 9 e^|x|/2 - e^-|x|/2 from the pair e^|x| = 2^k P (__libc_fast_expf_pair), the leading difference by a two-sum;
 * beyond, P 2^(k - 1) alone (e^-2|x| is below 2^-26). Below 1.5 ulp to nearest (the trigonometric note, section 10);
 * float only, round to nearest only as the layer (D57). C23 F.10.2.5: +-0 and +-infinity themselves, overflow from 89.42.
 */
float sinhf(float x)
{
	float a = __builtin_fabsf(x);
	float z;
	float hi;
	float lo;
	float r;
	float r_lo;
	float error;
	float h;
	int k;

	if (isnan(x) || isinf(x))
		return x + x;
	/* x^3/6 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	if (a > 90.0f)
		return x * 0x1p127f;
	if (a < 1.0f) {
		/* x + x z S(z): the next term, x^13/13!, is below 2^-32 of the result */
		z = x * x;
		return x + x * z * __builtin_fmaf(z, __builtin_fmaf(z, __builtin_fmaf(z, __builtin_fmaf(z, 0x1.ae6456p-26f, 0x1.71de3ap-19f), 0x1.a01a02p-13f), 0x1.111112p-7f), 0x1.555556p-3f);
	}
	hi = __libc_fast_expf_pair(a, &k, &lo);
	if (a > 9.0f) {
		/* 2^(k - 1) in two exact steps: k reaches 130, and the product overflows past FLT_MAX */
		h = (hi + lo) * __libc_float_from((uint32_t)(k - 1 < 127 ? k - 1 + 127 : 254) << 23);
		if (k - 1 > 127)
			h *= __libc_float_from((uint32_t)(k - 1 - 127 + 127) << 23);
		return x < 0 ? -h : h;
	}
	/* 1/(4H) for H = 2^(k - 1) P as 2^(-k - 1) / P, the quotient's remainder by a fused multiply-add */
	r = 1.0f / hi;
	r_lo = (__builtin_fmaf(-r, hi, 1.0f) - r * lo) / hi;
	h = __libc_float_from((uint32_t)(k - 1 + 127) << 23);
	r *= __libc_float_from((uint32_t)(127 - k - 1) << 23);
	r_lo *= __libc_float_from((uint32_t)(127 - k - 1) << 23);
	hi = __libc_fast_two_sum(hi * h, -r, &error);
	hi = hi + (error + (lo * h - r_lo));

	return x < 0 ? -hi : hi;
}
