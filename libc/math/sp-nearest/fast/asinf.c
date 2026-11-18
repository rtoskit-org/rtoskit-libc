/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast asinf of the single-precision FPU cores (PLAN D162, D220, D235): up to 1/2 |x| + |x|^3 P(x^2)
 * (__libc_fast_asinf_poly, degree 4); above, pi/2 - 2 asin(s) with s = sqrt((1 - |x|)/2) as a pair, the difference by Fast2Sum;
 * the sign of x. One call of the polynomial, so that it is inlined. Within D220's 0.9-ulp target to nearest over every
 * float (the float trigonometric note, section 27); round to nearest only as the layer (D57). C23 F.10.1.2: |x| > 1
 * invalid, +-0 itself.
 */
float asinf(float x)
{
	float a = __builtin_fabsf(x);
	float t = a;
	float t_lo = 0.0f;
	float hi;
	float lo;

	/* NaN (a quiet comparison), or |x| > 1: invalid */
	if (!islessequal(a, 1.0f))
		return isnan(x) ? x + x : __libc_math_invalidf();
	/* x^3/6 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	if (a > 0.5f)
		t = __libc_fast_half_angle(a, &t_lo);
	hi = __libc_fast_asinf_poly(t, t_lo, &lo);
	if (a > 0.5f)
		hi = __libc_fast_pair_from_larger(0x1.921fb6p+0f, -0x1.777a5cp-25f, 2.0f * hi, 2.0f * lo, &lo);
	hi = hi + lo;

	return x < 0 ? -hi : hi;
}
