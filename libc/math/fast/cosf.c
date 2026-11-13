/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast cosf of the double-FPU cores (PLAN D162, D175): for |x| < 2^20, x = k pi/2 + r in double
 * (__libc_fast_rem_pio2, r to 2^-52 relative) and sin((k + 1) pi/2 + r) by the Taylor polynomials in double; rounded once to float, below 0.51 ulp to nearest (the note,
 * section 3). Beyond 2^20, the infinities and NaNs, the double cos's slow path rounded once (__libc_fast_trig_large).
 */
float cosf(float x)
{
	double r;
	int k;

	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (!isless(__builtin_fabsf(x), 0x1p20f))
		return (float)__libc_fast_trig_large(x, 1);

	k = __libc_fast_rem_pio2((double)x, &r);

	return (float)__libc_fast_sin_quadrant(k + 1, r);
}
