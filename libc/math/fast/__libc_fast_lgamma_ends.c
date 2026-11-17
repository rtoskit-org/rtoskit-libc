/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_lgamma_ends.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The ends (C23 7.12.8.3, F.10.5.3): NaN returns itself, +-inf +inf, +-0 +inf with divide-by-zero; below 2^-60 in
 * magnitude, -log|x| (lgamma(1 + x) is below 2^-60 of it); from 2^60, x (log x - 1) as the strict file forms it
 * (Stirling's other terms are below 2^-60 relatively), overflowing past 2.5e305; the negative ones from 2^60 are poles
 */
double __libc_fast_lgamma_ends(double x)
{
	struct __libc_dd value;
	int exponent;
	int mode;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __builtin_fabs(x);
	if (x == 0 || x <= -0x1p60)
		return __libc_math_divbyzero(1.0);
	if (__builtin_fabs(x) < 0x1p-60)
		return -log(__builtin_fabs(x));

	exponent = ilogb(x);
	mode = __libc_math_nearest();
	value = __libc_dd_multiply(__libc_dd_add(__libc_log_core(x), (struct __libc_dd){ .hi = -1.0, .lo = 0.0 }), (struct __libc_dd){ .hi = scalbn(x, -exponent), .lo = 0.0 });
	__libc_math_restore(mode);

	return __libc_exp_scale(value, exponent);
}
