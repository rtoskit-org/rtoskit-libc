/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atanf of the single-precision FPU cores (PLAN D162, D176): atan |x| by the table kernel (__libc_fast_atanf_kernel), as pi/2 - atan(1/|x|) above 1, with the sign of x. Below 1 ulp to nearest (the trigonometric
 * note, section 7); float only, round to nearest only as the layer (D57). C23 F.10.1.3: +-infinity gives +-pi/2, +-0 itself.
 */
float atanf(float x)
{
	float a = __builtin_fabsf(x);
	float hi;
	float lo;

	if (isnan(x))
		return x + x;
	/* x^3/3 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;

	if (a > 1.0f) {
		/* pi/2 - atan(1/a), 1/a as a pair (its remainder by a fused multiply-add); +-infinity gives +-pi/2 */
		float u = 1.0f / a;

		hi = __libc_fast_atanf_kernel(u, isinf(a) ? 0.0f : __builtin_fmaf(-u, a, 1.0f) / a, &lo);
		hi = __libc_fast_pair_from(0x1.921fb6p+0f, -0x1.777a5cp-25f, hi, lo, &lo);
	} else {
		hi = __libc_fast_atanf_kernel(a, 0.0f, &lo);
	}
	hi = hi + lo;

	return x < 0 ? -hi : hi;
}
