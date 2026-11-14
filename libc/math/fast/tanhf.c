/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast tanhf of the double-FPU cores (PLAN D162, D177): m/(m + 2) with m = e^(2|x|) - 1 in double (the series below 1/8,
 * __libc_fast_exp_double above), the sign of x; rounded once to float, below 0.51 ulp to nearest (the trigonometric note,
 * section 9). C23 F.10.2.6: +-0 itself, +-infinity +-1.
 */
float tanhf(float x)
{
	double a = __builtin_fabs((double)x);
	double m;
	double r;

	if (isnan(x))
		return x + x;
	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, -x);
	/* From 9.01 tanh rounds to +-1; the infinities included, as the double function (inexact but for them) */
	if (!isless(a, 9.5))
		return (float)tanh((double)x);
	/* e^(2a) - 1 without cancellation: the series near 0, else e^(2a) from 2^(1/8) up, where the subtraction loses 3 bits */
	m = 2.0 * a <= 0.125 ? __libc_fast_expm1_series(2.0 * a) : __libc_fast_exp_double(2.0 * a) - 1.0;
	r = m / (m + 2.0);

	return (float)(x < 0 ? -r : r);
}
