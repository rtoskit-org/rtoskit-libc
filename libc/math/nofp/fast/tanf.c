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
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast tanf of the cores without an FPU (PLAN D162, D180): sinf's and cosf's kernel results from one reduction, divided
 * as nofp/speed's tanf divides them (D87), rounded to nearest without the rounding test; where either declines (within
 * 2^-14 of a multiple of pi/2) and from 2^20, the strict layer's float-float quotient (the trigonometric note, section 16).
 * C23 F.10.1.7: +-0 itself, +-infinity invalid.
 */
float tanf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	struct __libc_ff t;
	struct __libc_ff value;
	int64_t s;
	uint64_t sine;
	uint64_t cosine;
	int sine_exponent;
	int cosine_exponent;
	bool sine_negative;
	bool cosine_negative;
	int32_t cosine_m1;
	int32_t g;
	int k;

	/* The x^3 term is below half an ulp of x: the result rounds to x (keeping -0) */
	if (magnitude < 0x39000000)
		return x;
	if (magnitude < 0x49800000) {
		if (magnitude < 0x3c800000) {
			k = 0;
			s = (int64_t)((bits & 0x007fffff) | 0x00800000) << ((int)(magnitude >> 23) - 102);
			if ((bits >> 31) != 0)
				s = -s;
		} else {
			k = __libc_fixed_trig_reduce(bits, &s);
		}
		/* The terms of s once for both */
		__libc_fixed_trig_terms(s, &cosine_m1, &g);
		sine = __libc_fixed_trig_combine(k, s, cosine_m1, g, &sine_exponent, &sine_negative);
		cosine = __libc_fixed_trig_combine(k + 16, s, cosine_m1, g, &cosine_exponent, &cosine_negative);
		if (sine != 0 && cosine != 0)
			return __libc_fixed_trig_divide(sine, sine_exponent, sine_negative, cosine, cosine_exponent, cosine_negative);
	}

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	k = __libc_rem_pio2f(x, &t);
	value = __libc_ff_divide(__libc_sinf_core(k, t), __libc_sinf_core(k + 16, t));

	return value.hi + value.lo;
}
