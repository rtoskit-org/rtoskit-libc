/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast tanpif of the double-FPU cores (PLAN D162, D175): x = n/2 + y with n = round(2x) and y exact (|y| <= 1/4),
 * r = pi y in double (rounded once), then sin r / cos r (or -cos r / sin r for an odd n) in double; rounded once to float, below 0.51 ulp to nearest (the note, section 3).
 * The exact cases as the other build types: C23 F.10.1.16.
 */
float tanpif(float x)
{
	double t;
	double y;
	double r;
	double s;
	double c;
	int n;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	if (__builtin_fabsf(x) >= 0x1p24f)
		return copysignf(0.0f, x);
	if (__builtin_fabsf(x) >= 0x1p23f)
		return ((int32_t)x & 1) == 0 ? copysignf(0.0f, x) : copysignf(0.0f, -x);
	/* 2x = n + 2y exactly in double; n by the 1.5 * 2^52 shift */
	t = (double)x * 2.0 + 0x1.8p52;
	n = (int)(uint32_t)__libc_double_bits(t);
	y = (double)x - (t - 0x1.8p52) * 0.5;
	/* x a multiple of 1/2: +-0 for the integers, +-infinity with divide-by-zero between (a division at run time) */
	if (y == 0) {
		switch (n & 3) {
			case 0:
				return copysignf(0.0f, x);
			case 2:
				return copysignf(0.0f, -x);
			case 1:
				return 1.0f / fabsf((float)y);
			default:
				return -1.0f / fabsf((float)y);
		}
	}
	r = y * 0x1.921fb54442d18p+1;
	s = __libc_fast_sin_double(r);
	c = __libc_fast_cos_double(r);

	return (float)((n & 1) == 0 ? s / c : -c / s);
}
