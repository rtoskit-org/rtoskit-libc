/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast cosf of the cores without an FPU (PLAN D162, D180): sinf's path at k + 16 (the trigonometric note, section 16),
 * rounded to nearest without the rounding test; where the kernel declines (within 2^-14 of an odd multiple of pi/2) and
 * from 2^20, the strict layer's exact reduction and float-float core. C23 F.10.1.5: cos(+-0) = 1, +-infinity invalid.
 */
float cosf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	struct __libc_ff t;
	struct __libc_ff value;
	int64_t s;
	uint64_t v;
	int exponent;
	bool negative;
	int k;

	/* x^2/2 is below half an ulp of 1: 1 */
	if (magnitude < 0x39800000)
		return 1.0f;
	if (magnitude < 0x49800000) {
		if (magnitude < 0x3c800000) {
			k = 0;
			s = (int64_t)((bits & 0x007fffff) | 0x00800000) << ((int)(magnitude >> 23) - 102);
		} else {
			k = __libc_fixed_trig_reduce(bits, &s);
		}
		v = __libc_fixed_trig_kernel(k + 16, s, &exponent, &negative);
		if (v != 0)
			return __libc_fixed_trig_float(v, exponent, negative);
	}

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	k = __libc_rem_pio2f(x, &t);
	value = __libc_sinf_core(k + 16, t);

	return value.hi + value.lo;
}
