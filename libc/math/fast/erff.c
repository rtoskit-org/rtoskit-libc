/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erff.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast erff of the double-FPU cores (PLAN D162, D178): below 1/2 x P(x^2), above 1 - erfc|x| with
 * __libc_fast_erfc_double, the sign of x; rounded once to float, below 0.51 ulp to nearest (the trigonometric note,
 * section 12). C23 F.10.5.1: +-0 itself, +-infinity +-1.
 */
float erff(float x)
{
	double d = x;
	double a = __builtin_fabs(d);
	double r;

	if (isnan(x))
		return x + x;
	/* erfc(3.92) is below a quarter ulp of 1: +-1, as the other build types; the infinities included */
	if (!isless(a, 3.92))
		return x < 0 ? -1.0f : 1.0f;
	/* x P(x^2) is odd and exact at +-0; a subnormal x rounds once, with underflow */
	if (a < 0.5)
		return (float)__libc_fast_erf_small(d);
	r = 1.0 - __libc_fast_erfc_double(a);

	return (float)(x < 0 ? -r : r);
}
