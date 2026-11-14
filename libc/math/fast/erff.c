/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erff.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* Off the common path: a NaN, and from 3.92, where erfc is below a quarter ulp of 1, +-1 (the infinities included) */
[[gnu::noinline]] static float erff_unusual(float x)
{
	if (isnan(x))
		return x + x;
	return x < 0 ? -1.0f : 1.0f;
}

/*
 * The fast erff of the double-FPU cores (PLAN D162, D178, P11.06c; docs/research/libm-fast-float-trig.md section 22):
 * one integer test; below 1/2 x P(x^2) (__libc_fast_erf_small, odd, exact at +-0, a subnormal rounding once with
 * underflow); from 1/2 to 3.92 erf itself in eight pieces about c = round(2|x|)/2 (|x| - c exact, at most 1/4), degree 7
 * (__libc_fast_erf_pieces, 2^-30.7 absolute against erf above 0.52), no exponential; the sign of x by its bit. Rounded
 * once to float, below 0.51 ulp to nearest.
 */
float erff(float x)
{
	uint32_t ix = __libc_float_bits(x);
	uint32_t ia = ix & 0x7fffffffu;
	const double *c;
	double shifted;
	double a;
	double t;
	double p;

	if (ia >= 0x407ae148u)
		return erff_unusual(x);
	if (ia < 0x3f000000u)
		return (float)__libc_fast_erf_small((double)x);
	a = (double)__libc_float_from(ia);
	/* round(2|x|) by the 1.5 * 2^52 shift; the pieces 1 to 8 are rows 0 to 7 */
	shifted = a * 2.0 + 0x1.8p52;
	c = __libc_fast_erf_pieces[(uint32_t)__libc_double_bits(shifted) - 1u];
	t = a - (shifted - 0x1.8p52) * 0.5;
	p = c[7];
	for (int i = 6; i >= 0; --i)
		p = p * t + c[i];
	return __libc_float_from(__libc_float_bits((float)p) | (ix & 0x80000000u));
}
