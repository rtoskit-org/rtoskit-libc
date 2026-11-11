/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast asinpif of the double-FPU cores (PLAN D162, D176): atan(|x| / sqrt(1 - x^2)) with the sign of x in double with __libc_fast_atan_double (2^-43.5), times 1/pi;
 * rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 6). |x| > 1 and NaN as the double asinpi (C23 F.10.1.12).
 */
float asinpif(float x)
{
	double d = x;
	double s;
	double r;

	/* |x| > 1 (invalid), NaN: as the double function */
	if (!isless(__builtin_fabsf(x), 1.0f) && !(__builtin_fabsf(x) == 1.0f))
		return (float)asinpi(d);
	/* 1 - x^2 = (1 - x)(1 + x) in double: both factors exact */
	s = __builtin_sqrt((1.0 - d) * (1.0 + d));
	/* |x| = 1: pi/2 without the division by zero, which would raise divide-by-zero */
	r = (s == 0 ? 0x1.921fb54442d18p+0 : __libc_fast_atan_double(__builtin_fabs(d) / s)) * 0x1.45f306dc9c883p-2;

	return (float)(x < 0 || (x == 0 && signbit(x)) ? -r : r);
}
