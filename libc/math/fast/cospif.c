/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cospif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast cospif of the double-FPU cores (PLAN D162, D175): x = n/2 + y with n = round(2x) and y exact (|y| <= 1/4),
 * r = pi y in double (rounded once), then sin((n + 1) pi/2 + r) by the Taylor polynomials; rounded once to float, below 0.51 ulp to nearest (the note, section 3).
 * The exact cases as the other build types: C23 F.10.1.14.
 */
float cospif(float x)
{
	double t;
	double y;
	double r;
	int n;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* Every float from 2^24 up is even, and from 2^23 an integer */
	if (__builtin_fabsf(x) >= 0x1p24f)
		return 1.0f;
	if (__builtin_fabsf(x) >= 0x1p23f)
		return ((int32_t)x & 1) == 0 ? 1.0f : -1.0f;
	/* 2x = n + 2y exactly in double; n by the 1.5 * 2^52 shift */
	t = (double)x * 2.0 + 0x1.8p52;
	n = (int)(uint32_t)__libc_double_bits(t);
	y = (double)x - (t - 0x1.8p52) * 0.5;
	/* x a multiple of 1/2: n + 1/2 gives +0, an integer +-1 */
	if (y == 0)
		return (n & 1) != 0 ? 0.0f : (n & 3) == 0 ? 1.0f :
													-1.0f;
	r = y * 0x1.921fb54442d18p+1;

	return (float)__libc_fast_sin_quadrant(n + 1, r);
}
