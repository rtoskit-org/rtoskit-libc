/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cospif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast cospif of the single-precision FPU cores (PLAN D162, D175): 32 x = K + d exactly (|d| <= 1/2), s = d pi/32 as
 * float-float (the product's error by a fused multiply-add), then sin((K + 16) pi/32 + s) by the table kernel. Below 1 ulp to nearest (the note, section 4);
 * float only, round to nearest only as the layer (D57). The exact cases as the other build types: C23 F.10.1.14.
 */
float cospif(float x)
{
	float y;
	float d;
	float s_hi;
	float s_lo;
	float hi;
	float lo;
	int k;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* Every float from 2^24 up is even, and from 2^23 an integer */
	if (__builtin_fabsf(x) >= 0x1p24f)
		return 1.0f;
	if (__builtin_fabsf(x) >= 0x1p23f)
		return ((int32_t)x & 1) == 0 ? 1.0f : -1.0f;
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
	/* x a multiple of 1/2: n + 1/2 gives +0, an integer +-1 */
	if (d == 0 && (k & 15) == 0)
		return (k & 16) != 0 ? 0.0f : (k & 63) == 0 ? 1.0f :
													  -1.0f;
	s_hi = d * 0x1.921fb6p-4f;
	s_lo = __builtin_fmaf(d, 0x1.921fb6p-4f, -s_hi) + d * -0x1.777a5cp-29f;
	hi = __libc_fast_sinf_kernel(k + 16, s_hi, s_lo, &lo);

	return hi + lo;
}
