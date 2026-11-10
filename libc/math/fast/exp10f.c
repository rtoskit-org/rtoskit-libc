/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp10f of the double-FPU cores (PLAN D162, D168): 2^(x log2 10) in double (the product's error, 2^-53
 * relative, is far below a float ulp), rounded once to float, below 0.51 ulp to nearest. Special values as
 * C23 F.10.3.3.
 */
float exp10f(float x)
{
	if (isnan(x))
		return x + x;
	if (x > 39.0f)
		return x * 0x1p127f;
	if (x < -46.0f)
		return isinf(x) ? 0.0f : (float)__libc_math_underflow(1.0);

	return (float)__libc_fast_exp2((double)x * 0x1.a934f0979a371p+1);
}
