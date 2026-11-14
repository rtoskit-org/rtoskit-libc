/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * Off the common path: the tiny arguments as the other build types (x, with underflow for a subnormal one), and from
 * 2^20, the infinities and NaNs, the double sin's slow path rounded once (__libc_fast_trig_large)
 */
[[gnu::noinline]] static float sinf_unusual(float x)
{
	if (isless(__builtin_fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, -x);
	return (float)__libc_fast_trig_large(x, 0);
}

/*
 * The fast sinf of the double-FPU cores (PLAN D162, D175, P11.06c; docs/research/libm-fast-float-trig.md section 22):
 * for 2^-30 <= |x| < 2^20, one integer test, x = k pi/2 + r in double by two parts of pi/2 (__libc_fast_rem_pio2f) and
 * the float polynomials (__libc_fast_sinf_quadrant); rounded once to float, below 0.51 ulp to nearest.
 */
float sinf(float x)
{
	double r;
	int k;

	if ((__libc_float_bits(x) & 0x7fffffffu) - 0x30800000u >= 0x49800000u - 0x30800000u)
		return sinf_unusual(x);
	k = __libc_fast_rem_pio2f((double)x, &r);
	return (float)__libc_fast_sinf_quadrant(k, r);
}
