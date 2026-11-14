/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atanpif of the single-precision FPU cores (PLAN D162, D176): atan |x| as atanf with the sign of x; the pair times 1/pi rounded once (__libc_fast_pair_over_pi). Below 1 ulp to nearest (the trigonometric
 * note, section 7); float only, round to nearest only as the layer (D57). C23 F.10.1.13: +-infinity gives +-1/2, +-0 itself.
 */
float atanpif(float x)
{
	float a = __builtin_fabsf(x);
	float hi;
	float lo;

	if (isnan(x) || x == 0)
		return x + x;
	/*
	 * Far below 1, x / pi with x first scaled by 2^64, so that the constant's low part does not underflow (a subnormal
	 * product rounded twice measured 0.999 ulp), and one rounding back (x^3/3 is then below 2^-128 of x)
	 */
	if (a < 0x1p-64f) {
		float t = x * 0x1p64f;

		return __builtin_fmaf(t, 0x1.45f306p-2f, t * 0x1.b93910p-27f) * 0x1p-64f;
	}

	if (a > 1.0f) {
		/* pi/2 - atan(1/a), 1/a as a pair (its remainder by a fused multiply-add); +-infinity gives +-pi/2 */
		float u = 1.0f / a;

		hi = __libc_fast_atanf_kernel(u, isinf(a) ? 0.0f : __builtin_fmaf(-u, a, 1.0f) / a, &lo);
		hi = __libc_fast_pair_from(0x1.921fb6p+0f, -0x1.777a5cp-25f, hi, lo, &lo);
	} else {
		hi = __libc_fast_atanf_kernel(a, 0.0f, &lo);
	}
	hi = __libc_fast_pair_over_pi(hi, lo);

	return x < 0 ? -hi : hi;
}
