/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp2f of the double-FPU cores (PLAN D162, D168): 2^x in double, rounded once to float, below 0.51 ulp to
 * nearest and exact at the integers (r = 0); no hard-case correction. Special values as C23 F.10.3.2.
 */
float exp2f(float x)
{
	if (isnan(x))
		return x + x;
	if (x >= 128.0f)
		return x * 0x1p127f;
	if (x < -151.0f)
		return isinf(x) ? 0.0f : (float)__libc_math_underflow(1.0);

	return (float)__libc_fast_exp2((double)x);
}
