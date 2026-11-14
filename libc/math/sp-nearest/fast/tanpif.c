/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast tanpif of the single-precision FPU cores (PLAN D162, D175): 32 x = K + d exactly (|d| <= 1/2), s = d pi/32 as
 * float-float (the product's error by a fused multiply-add), then the kernel's sine and cosine divided (__libc_fast_divide_pairs), within the 1 ulp of tanpi. Below 1 ulp to nearest (the note, section 4);
 * float only, round to nearest only as the layer (D57). The exact cases as the other build types: C23 F.10.1.16.
 */
float tanpif(float x)
{
	float y;
	float d;
	float s_hi;
	float s_lo;
	float sine;
	float sine_lo;
	float cosine;
	float cosine_lo;
	int k;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	if (__builtin_fabsf(x) >= 0x1p24f)
		return copysignf(0.0f, x);
	if (__builtin_fabsf(x) >= 0x1p23f)
		return ((int32_t)x & 1) == 0 ? copysignf(0.0f, x) : copysignf(0.0f, -x);
	/*
	 * Far below 1, pi x with x first scaled by 2^64, so that the constant's low part does not underflow, and one rounding
	 * back; +-0 keeps its sign
	 */
	if (__builtin_fabsf(x) < 0x1p-90f) {
		float t = x * 0x1p64f;

		return __builtin_fmaf(t, 0x1.921fb6p+1f, t * -0x1.777a5cp-24f) * 0x1p-64f;
	}
	/* 32 x = K + d exactly: truncated, then brought to |d| <= 1/2 (|32 x| < 2^28) */
	y = 32.0f * x;
	k = (int)y;
	d = y - (float)k;
	if (d > 0.5f) {
		d -= 1.0f;
		++k;
	} else if (d < -0.5f) {
		d += 1.0f;
		--k;
	}
	/* x a multiple of 1/2: +-0 for the integers, +-infinity with divide-by-zero between (a division at run time) */
	if (d == 0 && (k & 15) == 0) {
		switch ((k & 63) / 16) {
			case 0:
				return copysignf(0.0f, x);
			case 2:
				return copysignf(0.0f, -x);
			case 1:
				return 1.0f / fabsf(d);
			default:
				return -1.0f / fabsf(d);
		}
	}
	s_hi = d * 0x1.921fb6p-4f;
	s_lo = __builtin_fmaf(d, 0x1.921fb6p-4f, -s_hi) + d * -0x1.777a5cp-29f;
	sine = __libc_fast_sinf_kernel(k, s_hi, s_lo, &sine_lo);
	cosine = __libc_fast_sinf_kernel(k + 16, s_hi, s_lo, &cosine_lo);

	return __libc_fast_divide_pairs(sine, sine_lo, cosine, cosine_lo);
}
