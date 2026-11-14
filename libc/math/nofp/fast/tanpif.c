/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast tanpif of the cores without an FPU (PLAN D162, D180): sinpif's and cospif's fixed-point results from one exact
 * split, divided as nofp/speed's tanf divides them (D87), rounded to nearest (the trigonometric note, section 16). The exact
 * zeros and poles as the strict file (divide-by-zero at the poles); below 2^-90 pi x rounded once. C23 F.10.1.14: +-0
 * itself, +-infinity invalid.
 */
float tanpif(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	uint64_t sine;
	uint64_t cosine;
	int sine_exponent;
	int cosine_exponent;
	bool sine_negative;
	bool cosine_negative;
	int32_t r;
	int32_t cosine_m1;
	int32_t g;
	int64_t s;
	uint64_t high;
	uint32_t low;
	int shift;
	int k;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	if (magnitude >= 0x4b800000)
		return copysignf(0.0f, x);
	if (magnitude >= 0x4b000000)
		return (bits & 1) == 0 ? copysignf(0.0f, x) : copysignf(0.0f, -x);
	if (magnitude < 0x12800000) {
		shift = ilogbf(x);
		return __libc_exp2f_scale(__libc_ff_scale(scalbnf(x, -shift), __LIBC_PIF), shift);
	}

	k = __libc_fixed_pi_split(bits, &r, &shift);
	if (r == 0 && (k & 15) == 0) {
		switch ((k & 63) / 16) {
			case 0:
				return copysignf(0.0f, x);
			case 2:
				return copysignf(0.0f, -x);
			case 1:
				/* +infinity with divide-by-zero: a division at run time, not a folded constant */
				return 1.0f / __builtin_fabsf(x - x);
			default:
				return -1.0f / __builtin_fabsf(x - x);
		}
	}
	/* s and its terms once for both; __libc_fixed_pi_small where the kernel declines (near a zero or a pole) */
	s = __libc_fixed_pi_s(r, shift, &high, &low);
	__libc_fixed_trig_terms(s, &cosine_m1, &g);
	sine = __libc_fixed_trig_combine(k, s, cosine_m1, g, &sine_exponent, &sine_negative);
	if (sine == 0)
		sine = __libc_fixed_pi_small(k, r, shift, high, low, &sine_exponent, &sine_negative);
	cosine = __libc_fixed_trig_combine(k + 16, s, cosine_m1, g, &cosine_exponent, &cosine_negative);
	if (cosine == 0)
		cosine = __libc_fixed_pi_small(k + 16, r, shift, high, low, &cosine_exponent, &cosine_negative);

	return __libc_fixed_trig_divide(sine, sine_exponent, sine_negative, cosine, cosine_exponent, cosine_negative);
}
