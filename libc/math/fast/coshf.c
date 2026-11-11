/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * coshf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast coshf of the double-FPU cores (PLAN D162, D177): (e^|x| + e^-|x|)/2 with __libc_fast_exp_double (2^-30.5), a sum
 * of positive terms; rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 9). C23 F.10.2.4:
 * cosh(+-0) = 1, +-infinity gives +infinity; overflow as the double cosh.
 */
float coshf(float x)
{
	double a = __builtin_fabs((double)x);
	double e;

	if (isnan(x))
		return x + x;
	/* cosh overflows float from 89.42; the infinities and the overflow as the double function */
	if (!isless(a, 89.5))
		return (float)cosh((double)x);
	e = __libc_fast_exp_double(a);

	return (float)(0.5 * (e + 1.0 / e));
}
