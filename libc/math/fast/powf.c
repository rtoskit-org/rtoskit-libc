/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * powf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>

#include "internal/math-fast.h"

/*
 * The fast powf of the double-FPU cores (PLAN D162, D170): the special cases of C23 F.10.4.5
 * (__libc_fast_powf_special), then 2^(y log2 |x|) in double: log2 |x| = e + log m / ln2 to 2^-39.5
 * (__libc_fast_log_double), the product's error below 2^-32 for |y log2 |x|| up to 150, and __libc_fast_exp2;
 * rounded once to float, below 0.51 ulp to nearest (the note, section 10).
 */
float powf(float x, float y)
{
	float result;
	bool negative;
	double sign;
	double t;
	double m;
	int e;

	if (__libc_fast_powf_special(x, y, &result, &negative))
		return result;

	sign = negative ? -1.0 : 1.0;
	m = __libc_fast_logf_split(fabsf(x), &e);
	t = (double)y * ((double)e + __libc_fast_log_double(m) * 0x1.71547652b82fep+0);
	/* Far out of range: the helpers give the mode's result and raise the exception (a constant would fold) */
	if (t > 1000.0)
		return (float)__libc_math_overflow(sign);
	if (t < -1000.0)
		return (float)__libc_math_underflow(sign);

	return (float)(sign * __libc_fast_exp2(t));
}
