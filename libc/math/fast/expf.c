/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast expf of the double-FPU cores (PLAN D162, D168): 2^(x / ln 2) in double, rounded once to float, below
 * 0.51 ulp to nearest; no hard-case correction. Overflow, underflow and the special values as C23 F.10.3.1.
 */
float expf(float x)
{
	if (isnan(x))
		return x + x;
	/* Past ln(FLT_MAX) the product overflows (+inf stays +inf, exactly) */
	if (x > 89.0f)
		return x * 0x1p127f;
	/* e^-inf is +0 exactly; below ln(2^-150) zero, or 2^-149 upward, raising underflow (a constant product would fold) */
	if (x < -104.0f)
		return isinf(x) ? 0.0f : (float)__libc_math_underflow(1.0);

	return (float)__libc_fast_exp2((double)x * 0x1.71547652b82fep+0);
}
