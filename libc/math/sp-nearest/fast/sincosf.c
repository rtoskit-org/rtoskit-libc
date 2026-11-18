/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* sincos and sincosf are GNU extensions */
#define _GNU_SOURCE

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast sincosf of the single-precision FPU cores (PLAN D162, D220, D234): one reduction (= q pi/2 + r with r as a float pair (__libc_fast_rem_pio2f_quadrant: three-piece Cody-Waite below 64, the layer's integer reduction above)), then both results from the same r (__libc_fast_sinf_pio2 for q and q + 1). As sinf
 * and cosf, 0.65 ulp to nearest (the float trigonometric note, section 26); round to nearest only as the layer (D57).
 */
void sincosf(float x, float *sinp, float *cosp)
{
	float r;
	float r_lo;
	int q;

	if (isnan(x) || isinf(x)) {
		*sinp = isnan(x) ? x + x : __libc_math_invalidf();
		*cosp = *sinp;
		return;
	}
	q = __libc_fast_rem_pio2f_quadrant(x, &r, &r_lo);
	/* sinf: the x^3 term is below half an ulp of x, so the result rounds to x (keeping -0) */
	*sinp = __builtin_fabsf(x) < 0x1p-12f ? x : __libc_fast_sinf_pio2(q, r, r_lo);
	*cosp = __libc_fast_sinf_pio2(q + 1, r, r_lo);
}
