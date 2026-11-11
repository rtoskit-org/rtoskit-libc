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

#include "internal/math-fast.h"

/*
 * The fast sincosf of the single-precision FPU cores (PLAN D162, D175): x = k pi/32 + s with s as float-float, by four-piece
 * Cody-Waite below 64 (__libc_fast_rem_pio32f) and by the layer's integer reduction above (__libc_rem_pio2f, k modulo 64);
 * s = x below 2^-6, then both from the table kernel (k and k + 16). Below 1 ulp to nearest (the note, section 4); float only, round to nearest only as the layer (D57).
 */
void sincosf(float x, float *sinp, float *cosp)
{
	struct __libc_ff s;
	float hi;
	float lo;
	int k;

	if (isnan(x) || isinf(x)) {
		*sinp = isnan(x) ? x + x : __libc_math_invalidf();
		*cosp = *sinp;
		return;
	}
	if (__builtin_fabsf(x) < 0x1p-6f) {
		k = 0;
		s = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	} else if (__builtin_fabsf(x) < 64.0f) {
		k = __libc_fast_rem_pio32f(x, &s.lo, &s.hi);
	} else {
		k = __libc_rem_pio2f(x, &s);
	}
	/* sinf: the x^3 term is below half an ulp of x, so the result rounds to x (keeping -0) */
	if (__builtin_fabsf(x) < 0x1p-12f) {
		*sinp = x;
	} else {
		hi = __libc_fast_sinf_kernel(k, s.hi, s.lo, &lo);
		*sinp = hi + lo;
	}
	hi = __libc_fast_sinf_kernel(k + 16, s.hi, s.lo, &lo);
	*cosp = hi + lo;
}
