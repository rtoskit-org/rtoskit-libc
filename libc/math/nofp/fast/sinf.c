/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast sinf of the cores without an FPU (PLAN D162, D180): for 2^-12 <= |x| < 2^20 the fixed-point reduction to
 * k pi/32 + s (x itself below 2^-6) and nofp/speed's kernel (D87), rounded to nearest without the rounding test: below
 * 0.5 + 2/512 ulp. Where the kernel declines (within 2^-14 of a multiple of pi) and from 2^20, the strict layer's exact
 * reduction and float-float core (the trigonometric note, section 16). C23 F.10.1.6: +-0 itself, +-infinity invalid.
 */
float sinf(float x)
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

	/* The x^3 term is below half an ulp of x: the result rounds to x (keeping -0) */
	if (magnitude < 0x39800000)
		return x;
	if (magnitude < 0x49800000) {
		if (magnitude < 0x3c800000) {
			/* x itself in units of 2^-48: m 2^(e + 25), exactly */
			k = 0;
			s = (int64_t)((bits & 0x007fffff) | 0x00800000) << ((int)(magnitude >> 23) - 102);
			if ((bits >> 31) != 0)
				s = -s;
		} else {
			k = __libc_fixed_trig_reduce(bits, &s);
		}
		v = __libc_fixed_trig_kernel(k, s, &exponent, &negative);
		if (v != 0)
			return __libc_fixed_trig_float(v, exponent, negative);
	}

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	k = __libc_rem_pio2f(x, &t);
	value = __libc_sinf_core(k, t);

	return value.hi + value.lo;
}
