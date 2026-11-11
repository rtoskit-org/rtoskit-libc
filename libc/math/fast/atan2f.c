/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atan2f of the double-FPU cores (PLAN D162, D176): atan(|y| / |x|), pi minus it for a negative x, with the sign of y in double with __libc_fast_atan_double (2^-43.5);
 * rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 6). The zeros, infinities and NaNs as the double atan2.
 */
float atan2f(float y, float x)
{
	double q;
	double r;

	/* The zeros, infinities and NaNs as the double function: C23 F.10.1.4's signed cases */
	if (x == 0 || y == 0 || !isfinite(x) || !isfinite(y))
		return (float)atan2((double)y, (double)x);
	q = __builtin_fabs((double)y) / __builtin_fabs((double)x);
	/*
	 * A tiny quotient with x > 0 (atan2 is q - q^3/3) nudged toward zero by 2^-40 relative, as the other build types,
	 * so the conversion rounds in the current direction and raises underflow for a subnormal result
	 */
	if (x > 0 && q < 0x1p-30) {
		r = q - q * 0x1p-40;
		return (float)(y < 0 ? -r : r);
	}
	r = __libc_fast_atan_double(q);
	if (x < 0)
		r = 0x1.921fb54442d18p+1 - r;

	return (float)(y < 0 ? -r : r);
}
