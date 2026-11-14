/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* A NaN, overflow beyond 89.0f, and a zero or 2^-149 below -104.0f (raising underflow; a constant product would fold) */
[[gnu::noinline]] static float expf_special(float x)
{
	if (isnan(x))
		return x + x;
	if (x > 0)
		return x * 0x1p127f;
	return isinf(x) ? 0.0f : (float)__libc_math_underflow(1.0);
}

/*
 * The fast expf of the double-FPU cores (PLAN D162, D168, P11.06b): 2^(x / ln 2) in double, rounded once to float, below
 * 0.51 ulp to nearest; no hard-case correction. One integer test sends |x| from 88 off the common path; the
 * arguments there between -104.0f and 89.0f come back to it, by quiet comparisons, which a NaN must not trip (C23 F.10.3.1).
 */
float expf(float x)
{
	if (__builtin_expect((__libc_float_bits(x) & 0x7fffffffu) >= 0x42b00000u, 0) && !(islessequal(x, 89.0f) && isgreaterequal(x, -104.0f)))
		return expf_special(x);
	return (float)__libc_fast_exp2((double)x * 0x1.71547652b82fep+0);
}
