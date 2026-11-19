/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast acosf of the double path (PLAN D162, D244; docs/research/libm-fast-double.md, section 39), in plain double:
 * pi/2 - (x + x z P(z)) below 1/2, and from 1/2 2 asin(sqrt((1 - |x|)/2)) or pi minus it (x = 1 gives sqrt 0 and +0
 * in every direction), with __libc_fast_asinf_series's one call site; rounded once to float. C23 F.10.1.1: |x| > 1 and NaN as the
 * double acos.
 */
float acosf(float x)
{
	uint32_t bits = __libc_float_bits(x) & 0x7fffffff;
	bool small = bits < 0x3f000000;
	double d = x;
	double y;
	double v;
	double r;

	/* |x| > 1 (invalid), NaN: as the double function */
	if (bits > 0x3f800000)
		return (float)acos(d);
	/* (1 - |x|)/2 exact from 1/2 */
	y = small ? d * d : (1.0 - __builtin_fabs(d)) * 0.5;
	v = small ? d : __builtin_sqrt(y);
	r = __libc_fast_asinf_series(v, y);
	if (small)
		return (float)(0x1.921fb54442d18p+0 - r);
	/* fabs: x = 1 gives 1 - 1 = -0 rounding downward, and acos 1 is +0 in every direction */
	if (x > 0)
		return (float)__builtin_fabs(2.0 * r);

	return (float)(0x1.921fb54442d18p+1 - 2.0 * r);
}
