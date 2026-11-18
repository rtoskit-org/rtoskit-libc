/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast cosf of the single-precision FPU cores (PLAN D162, D220, D234): x = q pi/2 + r with r as a float pair (__libc_fast_rem_pio2f_quadrant: three-piece Cody-Waite below 64, the layer's integer reduction above), then
 * cos(x) = sin(x + pi/2) from the next quadrant (__libc_fast_sinf_pio2). 0.65 ulp to nearest over every float (the
 * float trigonometric note, section 26); round to nearest only as the layer (D57).
 */
float cosf(float x)
{
	float r;
	float r_lo;
	int q;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	q = __libc_fast_rem_pio2f_quadrant(x, &r, &r_lo);

	return __libc_fast_sinf_pio2(q + 1, r, r_lo);
}
