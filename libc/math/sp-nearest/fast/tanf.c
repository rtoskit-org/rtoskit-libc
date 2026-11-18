/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * tan r for r = r_hi + r_lo, |r| <= pi/4, as the unrounded pair r_hi + *lo: tan r = r + r^3 T(z), z = r_hi^2, with T of
 * degree 6 (fitted, relative error 2^-27.7 of the result with float coefficients), r^3 as a pair and its leading term
 * r^3 T0 inside one fused multiply-add, as __libc_fast_sinf_pair, and r_lo with the derivative's slope
 */
static inline float tan_pair(float r_hi, float r_lo, float *lo)
{
	float z = r_hi * r_hi;
	float z_lo = __builtin_fmaf(r_hi, r_hi, -z);
	float cube = r_hi * z;
	float cube_lo = __builtin_fmaf(r_hi, z, -cube) + r_hi * z_lo;
	float t = __builtin_fmaf(z, __builtin_fmaf(z, __builtin_fmaf(z, __builtin_fmaf(z, __builtin_fmaf(z, 0x1.1f15d0p-8f, 0x1.53476cp-14f), 0x1.634a12p-7f), 0x1.5ca52ap-6f), 0x1.badce4p-5f), 0x1.110d88p-3f);

	float tail = __builtin_fmaf(cube, 0x1.555560p-2f, __builtin_fmaf(cube * z, t, cube_lo * 0x1.555560p-2f));
	float a = r_hi + tail;

	/* r_lo with the slope 1 + tan^2 r (up to 2 near pi/4; with slope 1 it cost 2^-24.3 of the result) */
	*lo = __builtin_fmaf(r_lo, __builtin_fmaf(a, a, 1.0f), tail);
	return r_hi;
}

/*
 * The fast tanf of the single-precision FPU cores (PLAN D162, D220, D234): x = q pi/2 + r with r as a float pair
 * (__libc_fast_rem_pio2f_quadrant: three-piece Cody-Waite below 64, the layer's integer reduction above), tan r as an
 * unrounded pair (tan_pair), and in the odd quadrants -1/tan r with the reciprocal's remainder by a fused multiply-add.
 * Within tan's 1.5 ulp to nearest over every float (D163; the float trigonometric note, section 26); round to nearest
 * only as the layer (D57).
 */
float tanf(float x)
{
	float r;
	float r_lo;
	float hi;
	float lo;
	float t;
	float t_lo;
	float q;
	int quadrant;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* The x^3 term is below half an ulp of x: the result rounds to x (keeping -0) */
	if (__builtin_fabsf(x) < 0x1p-13f)
		return x;
	quadrant = __libc_fast_rem_pio2f_quadrant(x, &r, &r_lo);
	hi = tan_pair(r, r_lo, &lo);
	if ((quadrant & 1) == 0)
		return hi + lo;
	/* -1/(t + t_lo): q = 1/t, then q + q (1 - q t - q t_lo), 1 - q t exact by a fused multiply-add */
	t = hi + lo;
	t_lo = (hi - t) + lo;
	q = 1.0f / t;
	return -__builtin_fmaf(q, __builtin_fmaf(-q, t, 1.0f) - q * t_lo, q);
}
