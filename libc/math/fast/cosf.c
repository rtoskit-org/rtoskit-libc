/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* Off the common path: from 2^20, the infinities and NaNs, the double cos's slow path rounded once */
[[gnu::noinline]] static float cosf_unusual(float x)
{
	return (float)__libc_fast_trig_large(x, 1);
}

/*
 * The fast cosf of the double-FPU cores (PLAN D162, D175, P11.06c; docs/research/libm-fast-float-trig.md section 22):
 * for |x| < 2^20, one integer test, x = k pi/2 + r in double by two parts of pi/2 (__libc_fast_rem_pio2f) and the float
 * polynomials; rounded once to float, below 0.51 ulp to nearest.
 */
float cosf(float x)
{
	double r;
	int k;

	if ((__libc_float_bits(x) & 0x7fffffffu) >= 0x49800000u)
		return cosf_unusual(x);
	k = __libc_fast_rem_pio2f((double)x, &r);
	return (float)__libc_fast_sinf_quadrant(k + 1, r);
}
