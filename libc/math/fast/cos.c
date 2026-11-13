/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cos.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast cos of the double-FPU cores (PLAN D162, D196; docs/research/libm-fast-double.md, section 14): cos x =
 * sin(|x| + pi/2), so sin's kernel with n + 16. C23 7.12.4.5, F.10.1.5: an infinity is a domain error; cos(+-0) is 1.
 */
double cos(double x)
{
	double a = __builtin_fabs(x);
	uint32_t n;
	double t;
	double t_lo;
	double z;
	double hi;
	double lo;

	/* 1, inexact unless x is 0, as the strict file */
	if (isless(a, 0x1p-27)) {
		volatile double tiny = 0x1p-80;

		return x == 0 ? 1.0 : 1.0 - tiny;
	}
	if (!isless(a, 0x1p20)) {
		if (isnan(x))
			return x + x;
		if (isinf(x))
			return __libc_math_invalid();
		return __libc_fast_trig_slow(x, 1);
	}

	n = __libc_fast_rem_pio32(a, &t, &t_lo) + 16;
	if (__libc_fast_trig_near_zero(n, t))
		return __libc_fast_trig_slow(x, 1);

	z = t * t;
	hi = __libc_fast_sin_turn(n, t, t_lo, __libc_fast_sin_poly(z), __libc_fast_cos_poly(z), &lo);

	return __libc_fast_signed_sum(hi, lo, (n & 32) != 0);
}
