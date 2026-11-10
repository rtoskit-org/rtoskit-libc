/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10m1f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp10m1f of the double-FPU cores (PLAN D162, D171): u = x ln10, e^u - 1 in double: within 1/8 of 0 the
 * series (__libc_fast_expm1_series, relative error 2^-30.3 however small u is), beyond it 2^(x log2 10) - 1
 * (__libc_fast_exp2, 2^-30.5 of 2^t, up to 2^-28 of a result 0.11 from 0); rounded once to float, below 0.55 ulp to
 * nearest (the note, section 12). Overflow and the special values as C23 F.10.3.3.
 */
float exp10m1f(float x)
{
	double u;

	if (isnan(x))
		return x + x;
	if (x > 39.0f)
		return x * 0x1p127f;
	/* 10^x is below a quarter of -1's ulp below 1: -1 */
	if (x < -8.0f)
		return -1.0f + 0x1p-100f;
	u = (double)x * 0x1.26bb1bbb55516p+1;
	if (__builtin_fabs(u) <= 0.125)
		return (float)__libc_fast_expm1_series(u);

	return (float)(__libc_fast_exp2((double)x * 0x1.a934f0979a371p+1) - 1.0);
}
