/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinpif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast sinpif of the cores without an FPU (PLAN D162, D180): 32x = K + d exactly in integers, s = d pi/32 to the
 * fixed-point kernel at K, or at sinpi's zeros, where it declines, pi d/32 itself from d's significand (the trigonometric
 * note, section 16); rounded to nearest. The exact zeros and ones as the strict file; below 2^-90 pi x rounded once.
 * C23 F.10.1.13: +-0 itself, +-infinity invalid.
 */
float sinpif(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	uint64_t v;
	int32_t r;
	int shift;
	int exponent;
	bool negative;
	int k;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* Every float from 2^23 up is an integer: sin(pi n) is +-0 with the sign of x */
	if (magnitude >= 0x4b000000)
		return copysignf(0.0f, x);
	if (magnitude < 0x12800000) {
		shift = ilogbf(x);
		return __libc_exp2f_scale(__libc_ff_scale(scalbnf(x, -shift), __LIBC_PIF), shift);
	}

	k = __libc_fixed_pi_split(bits, &r, &shift);
	/* x a multiple of 1/2: an integer gives +-0 with the sign of x, n + 1/2 gives +-1 */
	if (r == 0 && (k & 15) == 0)
		return (k & 16) == 0 ? copysignf(0.0f, x) : (k & 63) == 16 ? 1.0f :
																	 -1.0f;
	v = __libc_fixed_sinpi(k, r, shift, &exponent, &negative);

	return __libc_fixed_trig_float(v, exponent, negative);
}
