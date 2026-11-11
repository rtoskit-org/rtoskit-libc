/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast asinhf of the double-FPU cores (PLAN D162, D177): below 1/4 log1p(|x| + x^2/(1 + sqrt(1 + x^2))) (the argument
 * as accurate relative to |x| as the double operations), above log(|x| + sqrt(x^2 + 1)) (x^2 does not overflow a double),
 * the sign of x; rounded once to float, below 0.51 ulp to nearest (the trigonometric note, section 9). C23 F.10.2.2:
 * +-0 and +-infinity themselves.
 */
float asinhf(float x)
{
	double a = __builtin_fabs((double)x);
	double s;
	double r;

	if (isnan(x) || isinf(x))
		return x + x;
	/* The tiny arguments as the other build types: x, with underflow for a subnormal one */
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, -x);
	s = __builtin_sqrt(a * a + 1.0);
	/* log1p of an argument below 0.27: 1 + u in [sqrt(1/2), sqrt(2)) as __libc_fast_log1p_double needs */
	if (a < 0.25)
		r = __libc_fast_log1p_double(a + a * a / (1.0 + s));
	else
		r = __libc_fast_log_positive(a + s);

	return (float)(x < 0 ? -r : r);
}
