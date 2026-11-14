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
 * The fast atanf of the double-FPU cores (PLAN D162, D176): atan |x| with the sign of x in double with __libc_fast_atan_double (2^-43.5);
 * rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 6). +-infinity and NaN as the double atan (C23 F.10.1.3).
 */
float atanf(float x)
{
	double r;

	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, -x);
	if (isnan(x))
		return x + x;
	r = __libc_fast_atan_double(__builtin_fabs((double)x));

	return (float)(x < 0 ? -r : r);
}
