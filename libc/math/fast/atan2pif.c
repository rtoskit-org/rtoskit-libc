/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2pif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atan2pif of the double-FPU cores (PLAN D162, D176): atan(|y| / |x|), pi minus it for a negative x, with the sign of y in double with __libc_fast_atan_double (2^-43.5), times 1/pi;
 * rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 6). The zeros, infinities and NaNs as the double atan2pi.
 */
float atan2pif(float y, float x)
{
	double r;

	/* The zeros, infinities and NaNs as the double function: C23 F.10.1.14's signed cases */
	if (x == 0 || y == 0 || !isfinite(x) || !isfinite(y))
		return (float)atan2pi((double)y, (double)x);
	r = __libc_fast_atan_double(__builtin_fabs((double)y) / __builtin_fabs((double)x));
	if (x < 0)
		r = 0x1.921fb54442d18p+1 - r;
	r *= 0x1.45f306dc9c883p-2;

	return (float)(y < 0 ? -r : r);
}
