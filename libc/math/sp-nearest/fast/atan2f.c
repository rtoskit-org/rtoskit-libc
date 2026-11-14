/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atan2f of the single-precision FPU cores (PLAN D162, D176): atan(|y|/|x|) or pi/2 - atan(|x|/|y|) by the table kernel, the quotient as a pair, pi minus it for a negative x, the sign of y. Below 1 ulp to nearest (the trigonometric
 * note, section 7); float only, round to nearest only as the layer (D57). The zeros, infinities and NaNs as C23 F.10.1.4 (the other build types' cases).
 */
float atan2f(float y, float x)
{
	float ax;
	float ay;
	float hi;
	float lo;

	if (isnan(x) || isnan(y))
		return x + y;
	if (y == 0)
		return signbit(x) ? copysignf(0x1.921fb6p1f, y) : y;
	if (x == 0)
		return copysignf(0x1.921fb6p0f, y);
	if (isinf(y))
		return copysignf(isinf(x) ? (x > 0 ? 0x1.921fb6p-1f : 0x1.2d97c8p1f) : 0x1.921fb6p0f, y);
	if (isinf(x))
		return x > 0 ? copysignf(0.0f, y) : copysignf(0x1.921fb6p1f, y);
	ax = __builtin_fabsf(x);
	ay = __builtin_fabsf(y);
	/*
	 * A small argument scaled by 2^64 with the other (both exact), so that the quotient's remainder does not fall below
	 * the subnormals (it measured 1.4 ulp); with the other from 2^60 the quotient is below 2^-160 and needs no remainder
	 */
	if ((ax < 0x1p-100f || ay < 0x1p-100f) && ax < 0x1p60f && ay < 0x1p60f) {
		ax *= 0x1p64f;
		ay *= 0x1p64f;
	}
	if (ay <= ax) {
		/* atan(|y|/|x|), the quotient as a pair */
		float q = ay / ax;

		/*
		 * atan q = q (q^3/3 below 2^-120 of q), and q is the quotient rounded once: below 2^-102 its remainder would round
		 * on the subnormal grid, up to half an ulp of q, and adding it back measured 0.67 ulp (complex_float, clogf)
		 */
		if (q < 0x1p-60f && x > 0)
			return y < 0 ? -q : q;
		hi = __libc_fast_atanf_kernel(q, __builtin_fmaf(-q, ax, ay) / ax, &lo);
	} else {
		/* pi/2 - atan(|x|/|y|) */
		float q = ax / ay;

		hi = __libc_fast_atanf_kernel(q, __builtin_fmaf(-q, ay, ax) / ay, &lo);
		hi = __libc_fast_pair_from(0x1.921fb6p+0f, -0x1.777a5cp-25f, hi, lo, &lo);
	}
	if (x < 0)
		hi = __libc_fast_pair_from(0x1.921fb6p+1f, -0x1.777a5cp-24f, hi, lo, &lo);
	hi = hi + lo;

	return y < 0 ? -hi : hi;
}
