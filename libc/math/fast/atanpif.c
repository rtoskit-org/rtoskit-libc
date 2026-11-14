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
 * The fast atanpif of the double-FPU cores (PLAN D162, D176): atan |x| with the sign of x in double with __libc_fast_atan_double (2^-43.5), times 1/pi;
 * rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 6). NaN as the double atanpi (C23 F.10.1.13), +-infinity +-1/2 by the kernel.
 */
float atanpif(float x)
{
	double r;

	/* NaN, and +-0 with its sign in every direction (the kernel's t + t z P is -0 at 0 rounding downward) */
	if (isnan(x) || x == 0)
		return x + x;
	r = __libc_fast_atan_double(__builtin_fabs((double)x)) * 0x1.45f306dc9c883p-2;

	return (float)(x < 0 ? -r : r);
}
