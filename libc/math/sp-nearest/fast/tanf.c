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
 * The fast tanf of the single-precision FPU cores (PLAN D162, D175): x = k pi/32 + s with s as float-float, by four-piece
 * Cody-Waite below 64 (__libc_fast_rem_pio32f) and by the layer's integer reduction above (__libc_rem_pio2f, k modulo 64);
 * s = x below 2^-6, then the kernel's sine and cosine as unrounded pairs, divided with the quotient's remainder by a fused multiply-add (__libc_fast_divide_pairs). Below 1 ulp to nearest within the 1.5 ulp of tan (D163) (the note, section 4); float only, round to nearest only as the layer (D57).
 */
float tanf(float x)
{
	struct __libc_ff s;
	float sine;
	float sine_lo;
	float cosine;
	float cosine_lo;
	int k;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* The x^3 term is below half an ulp of x: the result rounds to x (keeping -0) */
	if (__builtin_fabsf(x) < 0x1p-13f)
		return x;
	if (__builtin_fabsf(x) < 0x1p-6f) {
		k = 0;
		s = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	} else if (__builtin_fabsf(x) < 64.0f) {
		k = __libc_fast_rem_pio32f(x, &s.lo, &s.hi);
	} else {
		k = __libc_rem_pio2f(x, &s);
	}
	sine = __libc_fast_sinf_kernel(k, s.hi, s.lo, &sine_lo);
	cosine = __libc_fast_sinf_kernel(k + 16, s.hi, s.lo, &cosine_lo);

	return __libc_fast_divide_pairs(sine, sine_lo, cosine, cosine_lo);
}
