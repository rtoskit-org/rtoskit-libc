/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* A NaN, overflow beyond 128.0f, and a zero or 2^-149 below -151.0f (raising underflow; a constant product would fold) */
[[gnu::noinline]] static float exp2f_special(float x)
{
	if (isnan(x))
		return x + x;
	if (x > 0)
		return x * 0x1p127f;
	return isinf(x) ? 0.0f : (float)__libc_math_underflow(1.0);
}

/*
 * The fast exp2f of the double-FPU cores (PLAN D162, D168, P11.06b): 2^x in double, rounded once to float, below
 * 0.51 ulp to nearest; no hard-case correction. One integer test sends |x| from 128 off the common path; the
 * arguments there between -151.0f and 128.0f come back to it, by quiet comparisons, which a NaN must not trip (C23 F.10.3.4).
 */
float exp2f(float x)
{
	if (__builtin_expect((__libc_float_bits(x) & 0x7fffffffu) >= 0x43000000u, 0) && !(islessequal(x, 128.0f) && isgreaterequal(x, -151.0f)))
		return exp2f_special(x);
	return (float)__libc_fast_exp2((double)x);
}
