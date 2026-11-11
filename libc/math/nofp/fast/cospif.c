/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cospif.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast cospif of the cores without an FPU (PLAN D162, D180): sinpif's path at K + 16 (the trigonometric note, section
 * 16), rounded to nearest; the exact zeros and ones as the strict file. C23 F.10.1.12: cospi(+-0) = 1, +-infinity invalid.
 */
float cospif(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	uint64_t v;
	int32_t r;
	int shift;
	int exponent;
	bool negative;
	int k;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* Every float from 2^24 up is even, and from 2^23 an integer of the significand's parity */
	if (magnitude >= 0x4b800000)
		return 1.0f;
	if (magnitude >= 0x4b000000)
		return (bits & 1) == 0 ? 1.0f : -1.0f;
	/* (pi x)^2/2 is below half an ulp of 1 (and far below, the split has no bits left): 1 */
	if (magnitude < 0x38000000)
		return 1.0f;

	k = __libc_fixed_pi_split(bits, &r, &shift);
	if (r == 0 && (k & 15) == 0)
		return (k & 16) != 0 ? 0.0f : (k & 63) == 0 ? 1.0f :
													  -1.0f;
	v = __libc_fixed_sinpi(k + 16, r, shift, &exponent, &negative);

	return __libc_fixed_trig_float(v, exponent, negative);
}
