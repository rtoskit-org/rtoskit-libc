/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast sinhf of the double-FPU cores (PLAN D162, D177): below 1/2 the odd Taylor series to x^11 in double, above
 * (e^|x| - e^-|x|)/2 with __libc_fast_exp_double (2^-30.5); rounded once to float, below 0.51 ulp to nearest (the
 * trigonometric note, section 9). C23 F.10.2.5: +-0 and +-infinity themselves; overflow as the double sinh.
 */
float sinhf(float x)
{
	double d = x;
	double a = __builtin_fabs(d);
	double z;
	double e;
	double r;

	if (isnan(x))
		return x + x;
	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, x);
	/* sinh overflows float from 89.42; the infinities and the overflow as the double function */
	if (!isless(a, 89.5))
		return (float)sinh(d);
	if (a < 0.5) {
		/* x + x^3/6 + ... + x^11/11!: the next term is below 2^-44 of the result */
		z = d * d;
		r = d + d * z * (0x1.5555555555555p-3 + z * (0x1.1111111111111p-7 + z * (0x1.a01a01a01a01ap-13 + z * (0x1.71de3a556c734p-19 + z * 0x1.ae64567f544e4p-26))));
		return (float)r;
	}
	e = __libc_fast_exp_double(a);
	r = 0.5 * (e - 1.0 / e);

	return (float)(x < 0 ? -r : r);
}
