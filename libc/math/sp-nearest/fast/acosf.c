/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast acosf of the single-precision FPU cores (PLAN D162, D176): up to 1/2 pi/2 - asin x; above, 2 asin(s) with s = sqrt((1 - |x|)/2) as a pair, or pi minus it for a negative x. Below 1 ulp to nearest (the trigonometric
 * note, section 7); float only, round to nearest only as the layer (D57). C23 F.10.1.1: |x| > 1 invalid, acos 1 = +0.
 */
float acosf(float x)
{
	float a = __builtin_fabsf(x);
	float s;
	float s_lo;
	float hi;
	float lo;

	if (isnan(x))
		return x + x;
	if (a > 1.0f)
		return __libc_math_invalidf();
	if (a <= 0.5f) {
		/* pi/2 - asin x */
		hi = __libc_fast_asinf_small(x, 0.0f, &lo);
		hi = __libc_fast_pair_from(0x1.921fb6p+0f, -0x1.777a5cp-25f, hi, lo, &lo);
	} else {
		/* 2 asin(s), s = sqrt((1 - |x|)/2), or pi minus it for a negative x */
		s = __libc_fast_half_angle(a, &s_lo);
		hi = __libc_fast_asinf_small(s, s_lo, &lo);
		hi *= 2.0f;
		lo *= 2.0f;
		if (x < 0)
			hi = __libc_fast_pair_from(0x1.921fb6p+1f, -0x1.777a5cp-24f, hi, lo, &lo);
	}

	return hi + lo;
}
