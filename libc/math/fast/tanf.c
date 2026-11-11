/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast tanf of the double-FPU cores (PLAN D162, D175): for |x| < 2^20, x = k pi/2 + r in double
 * (__libc_fast_rem_pio2, r to 2^-52 relative) and sin r / cos r (or -cos r / sin r for an odd k) in double; rounded once to float, below 0.51 ulp to nearest (the note,
 * section 3). Beyond 2^20, the infinities and NaNs, the double tan rounded once.
 */
float tanf(float x)
{
	double r;
	double s;
	double c;
	int k;

	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, x);
	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (!isless(__builtin_fabsf(x), 0x1p20f))
		return (float)tan((double)x);

	k = __libc_fast_rem_pio2((double)x, &r);
	s = __libc_fast_sin_double(r);
	c = __libc_fast_cos_double(r);

	return (float)((k & 1) == 0 ? s / c : -c / s);
}
