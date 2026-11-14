/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* sincos and sincosf are GNU extensions */
#define _GNU_SOURCE

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * Off the common path: a tiny x (sin x as the other build types, with underflow for a subnormal one; cos x exactly as
 * cosf computes it, through the reduction, whose k may be -1 for a tiny negative x rounding downward), and from 2^20,
 * the infinities and NaNs, the double functions' slow path rounded once
 */
[[gnu::noinline]] static void sincosf_unusual(float x, float *sinp, float *cosp)
{
	double r;
	int k;

	if (isless(__builtin_fabsf(x), 0x1p-30f)) {
		*sinp = __libc_math_tinyf(x, -x);
		k = __libc_fast_rem_pio2f((double)x, &r);
		*cosp = (float)__libc_fast_sinf_quadrant(k + 1, r);
		return;
	}
	*sinp = (float)__libc_fast_trig_large(x, 0);
	*cosp = (float)__libc_fast_trig_large(x, 1);
}

/*
 * The fast sincosf of the double-FPU cores (PLAN D162, D175, P11.06c; docs/research/libm-fast-float-trig.md section
 * 22): sinf's and cosf's paths with one reduction and one integer test, bit for bit their results.
 */
void sincosf(float x, float *sinp, float *cosp)
{
	double r;
	double s;
	double c;
	int k;

	if ((__libc_float_bits(x) & 0x7fffffffu) - 0x30800000u >= 0x49800000u - 0x30800000u) {
		sincosf_unusual(x, sinp, cosp);
		return;
	}
	k = __libc_fast_rem_pio2f((double)x, &r);
	s = __libc_fast_sinf_poly(r);
	c = __libc_fast_cosf_poly(r);
	if ((k & 1) != 0) {
		double t = s;

		s = c;
		c = -t;
	}
	if ((k & 2) != 0) {
		s = -s;
		c = -c;
	}
	*sinp = (float)s;
	*cosp = (float)c;
}
