/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10m1f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp10m1f of the single-precision FPU cores (PLAN D162, D171): k = round(x log2 10), r = x - k log10(2)f
 * exact by a fused multiply-add, then r + its low part as a fresh pair, u = (r + r_lo) ln10 as u_hi + u_lo; e^u 2^k - 1
 * by __libc_fast_expm1f_kernel. Below 1 ulp to nearest (the note, section 12); float only, round to nearest only as
 * the layer (D57).
 */
float exp10m1f(float x)
{
	float k;
	float r;
	float r_lo;
	float u;

	if (isnan(x))
		return x + x;
	if (x > 39.0f)
		return x * 0x1p127f;
	/* 10^x is below a quarter of -1's ulp below 1: -1 */
	if (x < -8.0f)
		return -1.0f + 0x1p-100f;
	/*
	 * Far below 1, x ln10 with x first scaled by 2^64, so that the constant's low part does not underflow, and one
	 * rounding back (the low terms' underflow cost 0.98 ulp near FLT_MIN, measured); +-0 keeps its sign
	 */
	if (__builtin_fabsf(x) < 0x1p-90f) {
		float t = x * 0x1p64f;

		/* The low product's zero has the other sign where the constant's low part is negative: -0 + +0 is +0 */
		if (x == 0)
			return x;
		return __builtin_fmaf(t, 0x1.26bb1cp+1f, t * -0x1.12aabap-25f) * 0x1p-64f;
	}

	k = __builtin_fmaf(x, 0x1.a934f0p+1f, 0x1.8p23f) - 0x1.8p23f;
	r = __builtin_fmaf(-k, 0x1.344136p-2f, x);
	/*
	 * r + k (log10(2)f - log10 2) as a fresh pair by a two-sum: the low part reaches 2^-21 at k = 24, and the
	 * kernel's u_lo (1 + u_hi) left a third of an ulp of it out (1.25 ulp at x = 7.07, measured)
	 */
	r = __libc_fast_two_sum(r, k * 0x1.ec10c0p-27f, &r_lo);
	u = r * 0x1.26bb1cp+1f;

	return __libc_fast_expm1f_kernel(u, __builtin_fmaf(r, 0x1.26bb1cp+1f, -u) + __builtin_fmaf(r, -0x1.12aabap-25f, r_lo * 0x1.26bb1cp+1f), (int)k);
}
