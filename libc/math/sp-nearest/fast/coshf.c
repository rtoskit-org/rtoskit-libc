/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * coshf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast coshf of the single-precision FPU cores (PLAN D162, D177): up to 9 e^|x|/2 + e^-|x|/2 from the pair
 * e^|x| = 2^k P (__libc_fast_expf_pair), a sum of positive terms, the leading sum by a two-sum; beyond, P 2^(k - 1) alone.
 * Below 1.5 ulp to nearest (the trigonometric note, section 10); float only, round to nearest only as the layer (D57).
 * C23 F.10.2.4: cosh(+-0) = 1, +-infinity gives +infinity, overflow from 89.42.
 */
float coshf(float x)
{
	float a = __builtin_fabsf(x);
	float hi;
	float lo;
	float r;
	float r_lo;
	float error;
	float h;
	int k;

	if (isnan(x))
		return x + x;
	if (a > 90.0f)
		return a * 0x1p127f;
	/* x^2/2 is below half an ulp of 1: 1, inexact but for x = 0 */
	if (a < 0x1p-12f)
		return 1.0f + a * a;
	hi = __libc_fast_expf_pair(a, &k, &lo);
	if (a > 9.0f) {
		/* 2^(k - 1) in two exact steps: k reaches 130, and the product overflows past FLT_MAX */
		h = (hi + lo) * __libc_float_from((uint32_t)(k - 1 < 127 ? k - 1 + 127 : 254) << 23);
		if (k - 1 > 127)
			h *= __libc_float_from((uint32_t)(k - 1 - 127 + 127) << 23);
		return h;
	}
	/* 1/(4H) for H = 2^(k - 1) P as 2^(-k - 1) / P, the quotient's remainder by a fused multiply-add */
	r = 1.0f / hi;
	r_lo = (__builtin_fmaf(-r, hi, 1.0f) - r * lo) / hi;
	h = __libc_float_from((uint32_t)(k - 1 + 127) << 23);
	r *= __libc_float_from((uint32_t)(127 - k - 1) << 23);
	r_lo *= __libc_float_from((uint32_t)(127 - k - 1) << 23);
	hi = __libc_fast_two_sum(hi * h, r, &error);

	return hi + (error + (lo * h + r_lo));
}
