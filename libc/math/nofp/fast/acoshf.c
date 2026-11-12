/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acoshf.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"
#include "internal/math.h"

/* (acosh(1 + t)/sqrt(2t) - 1)/t for t <= 1/4 by its Chebyshev fit of degree 5 in signed Q31 (t p(t) within 2^-34.1) */
static const int32_t series[6] = { -178956971, 40265312, -11983445, 4072923, -1462670, 435861 };

/*
 * The fast acoshf of the cores without an FPU (PLAN D162, D185), in fixed point and rounded to nearest once. Up to 1.25,
 * acosh(1 + t) = sqrt(2t) K(t), K = 1 + t p(t): t = x - 1 = w 2^-23 is exact, and so is 2t, whose square root comes from w
 * shifted by an even count to [2^30, 2^32); relative however close to 1. Above, log(x + sqrt(x^2 - 1)) with x^2 - 1 exact in
 * 64 bits (__libc_fixed_log_root), log(2x) from 2^12. C23 F.10.2.1: +0 at 1, +infinity itself, x < 1 invalid.
 */
float acoshf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint64_t v;
	uint32_t w;
	uint32_t r;
	int32_t up;
	int32_t p;
	int exponent;
	int shift;
	int z;

	if (bits <= 0x3f800000 || bits >= 0x7f800000) {
		if (bits == 0x3f800000)
			return 0.0f;
		return bits == 0x7f800000 || (bits > 0x7f800000 && bits < 0x80000000u) || bits > 0xff800000 ? x + x : __libc_math_invalidf();
	}
	if (bits > 0x3fa00000) {
		v = __libc_fixed_log_root(bits, true, &exponent);
		return __libc_fixed_float(v, exponent, false);
	}

	/* 2t = w 2^-22 = (w 2^shift) 2^-32 2^(10 - shift): sqrt(2t) = r 2^(-27 - shift/2) with r = sqrt of the shifted w in Q32 */
	w = (bits & __LIBC_FLOAT_FRACTION);
	shift = (int)stdc_leading_zeros(w) & ~1;
	r = __libc_fixed_square_root(w << shift);

	/* K(t) with t = w 2^9 in Q32 (at most 1/4) */
	p = series[5];
	for (int i = 4; i >= 0; --i)
		p = series[i] + __libc_fixed_high_signed((int32_t)(w << 9), p);
	up = (int32_t)(__libc_fixed_wide_signed((int32_t)(w << 9), p) >> 31);

	/* r K in units of 2^-31 of r, K in (0.98, 1] */
	v = ((uint64_t)r << 31) - (__libc_fixed_wide(r, -(uint32_t)up) >> 1);
	z = (int)stdc_leading_zeros(v);
	return __libc_fixed_float(v << z, 5 - shift / 2 - z, false);
}
