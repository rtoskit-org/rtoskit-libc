/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* sincos and sincosf are GNU extensions */
#define _GNU_SOURCE

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast sincosf of the double-FPU cores (PLAN D162, D175): for |x| < 2^20, x = k pi/2 + r in double
 * (__libc_fast_rem_pio2, r to 2^-52 relative) and both Taylor polynomials in double, sin and cos picked by the quadrant; rounded once to float, below 0.51 ulp to nearest (the note,
 * section 3). Beyond 2^20, the infinities and NaNs, the double sin's and cos's slow path rounded once (__libc_fast_trig_large).
 */
void sincosf(float x, float *sinp, float *cosp)
{
	double r;
	double s;
	double c;
	int k;

	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (!isless(__builtin_fabsf(x), 0x1p20f)) {
		*sinp = (float)__libc_fast_trig_large(x, 0);
		*cosp = (float)__libc_fast_trig_large(x, 1);
		return;
	}
	k = __libc_fast_rem_pio2((double)x, &r);
	s = __libc_fast_sin_double(r);
	c = __libc_fast_cos_double(r);
	/* sinf: the tiny arguments as the other build types */
	*sinp = isless(__builtin_fabsf(x), 0x1p-30f) ? __libc_math_tinyf(x, -x) : (float)((k & 2) == 0 ? ((k & 1) == 0 ? s : c) : ((k & 1) == 0 ? -s : -c));
	*cosp = (float)((k & 2) == 0 ? ((k & 1) == 0 ? c : -s) : ((k & 1) == 0 ? -c : s));
}
