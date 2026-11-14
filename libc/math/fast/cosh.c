/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* The ends: NaN returns itself and +-inf +inf; below 2^-27 1 + tiny (inexact); beyond 22 e^|x|/2 */
[[gnu::noinline]] static double cosh_ends(double x)
{
	double a = __builtin_fabs(x);

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return a;
	if (a < 0x1p-27) {
		/* At run time, so that 1 + tiny is inexact (the fast files drop -frounding-math); cosh(+-0) is exactly 1 */
		volatile double tiny = 0x1p-80;

		return x == 0 ? 1.0 : 1.0 + tiny;
	}

	return __libc_fast_exp_half(a, 1.0);
}

/*
 * The fast cosh of the double-FPU cores (PLAN D162, D201), in double with one rounding: (e^|x| + e^-|x|)/2 by the
 * hyperbolic kernel up to 22 (__libc_fast_sinhcosh, the halving in the scales; nothing cancels, so no series near 0:
 * there k = 0 and the result is 1 + E(x)), e^|x|/2 beyond (overflow above ln(2 DBL_MAX)). C23 7.12.5.4, F.10.2.4.
 */
double cosh(double x)
{
	double s_hi;
	double s_lo;
	double c_hi;
	double c_lo;

	/* 2^-27 <= |x| < 32 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x404) || __builtin_fabs(x) > 22.0)
		return cosh_ends(x);

	__libc_fast_sinhcosh(__builtin_fabs(x), 1, &s_hi, &s_lo, &c_hi, &c_lo);
	return c_hi + c_lo;
}
