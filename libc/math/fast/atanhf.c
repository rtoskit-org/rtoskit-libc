/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atanhf of the double-FPU cores (PLAN D162, D177): log((1 + |x|)/(1 - |x|))/2 in double, both terms exact, as
 * log1p(2|x|/(1 - |x|)) below 1/8 so that the relative accuracy holds near 0, the sign of x; rounded once to float, below
 * 0.51 ulp to nearest (the trigonometric note, section 9). C23 F.10.2.3: +-1 gives +-infinity with divide-by-zero, beyond
 * invalid, +-0 itself.
 */
float atanhf(float x)
{
	double a = __builtin_fabs((double)x);
	double r;

	/* |x| >= 1 (divide-by-zero or invalid) and NaN: as the double function */
	if (!isless(__builtin_fabsf(x), 1.0f))
		return (float)atanh((double)x);
	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, x);
	if (a < 0.125)
		r = 0.5 * __libc_fast_log1p_double(2.0 * a / (1.0 - a));
	else
		r = 0.5 * __libc_fast_log_positive((1.0 + a) / (1.0 - a));

	return (float)(x < 0 ? -r : r);
}
