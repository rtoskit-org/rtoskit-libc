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

#define _GNU_SOURCE

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/* One result from the combination or, where it declines and from 2^20, the strict layer's float-float core */
static float one(float x, bool fixed, int k, int64_t s, int32_t cosine_m1, int32_t g, int offset)
{
	struct __libc_ff t;
	struct __libc_ff value;
	uint64_t v;
	int exponent;
	bool negative;

	if (fixed) {
		v = __libc_fixed_trig_combine(k + offset, s, cosine_m1, g, &exponent, &negative);
		if (v != 0)
			return __libc_fixed_trig_float(v, exponent, negative);
	}
	k = __libc_rem_pio2f(x, &t);
	value = __libc_sinf_core(k + offset, t);

	return value.hi + value.lo;
}

/*
 * The fast sincosf of the cores without an FPU (PLAN D162, D180): sinf's and cosf's paths from one reduction (the
 * trigonometric note, section 16), the kernel's terms of s computed once, each result rounded to nearest or taken from
 * the float-float core where the kernel declines. NaN gives NaN twice, +-infinity invalid twice.
 */
void sincosf(float x, float *sinp, float *cosp)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	int64_t s = 0;
	int k = 0;
	int32_t cosine_m1 = 0;
	int32_t g = 0;

	if (isnan(x)) {
		*sinp = x + x;
		*cosp = x + x;
		return;
	}
	if (isinf(x)) {
		*sinp = __libc_math_invalidf();
		*cosp = __libc_math_invalidf();
		return;
	}
	/* sinf: the x^3 term is below half an ulp of x (keeping -0); cosf: x^2/2 below half an ulp of 1 */
	if (magnitude < 0x39800000) {
		*sinp = x;
		*cosp = 1.0f;
		return;
	}
	if (magnitude < 0x3c800000) {
		s = (int64_t)((bits & 0x007fffff) | 0x00800000) << ((int)(magnitude >> 23) - 102);
		if ((bits >> 31) != 0)
			s = -s;
	} else if (magnitude < 0x49800000) {
		k = __libc_fixed_trig_reduce(bits, &s);
	}
	/* The terms of s once for both */
	if (magnitude < 0x49800000)
		__libc_fixed_trig_terms(s, &cosine_m1, &g);
	*sinp = one(x, magnitude < 0x49800000, k, s, cosine_m1, g, 0);
	*cosp = one(x, magnitude < 0x49800000, k, s, cosine_m1, g, 16);
}
