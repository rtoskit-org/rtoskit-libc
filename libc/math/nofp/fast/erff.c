/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erff.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * erf(c + s/2) = a0 + a1 s + ... + a7 s^7 for -1/2 <= s < 1/2, c = 3/4 + j/2 the middle of [1/2 + j/2, 1 + j/2), in signed
 * Q31: Chebyshev fits in 256-bit arithmetic, each coefficient rounded; the rounded polynomial is within 2^-29.9 of erf
 * (docs/research/libm-fast-float-trig.md, section 17)
 */
static const int32_t pieces[7][8] = {
	{ 1527195094, 690341979, -258878166, 7190999, 20223341, -3571782, -891973, 301081 }, /* 3/4 */
	{ 1981912936, 253962621, -158726822, 44972561, -823025, -3166542, 691139, 62617 }, /* 5/4 */
	{ 2118861280, 56666720, -49583358, 24201426, -6456616, 444261, 303654, -99775 }, /* 7/4 */
	{ 2144342488, 7669007, -8627600, 5831635, -2561982, 715343, -93267, -12011 }, /* 9/4 */
	{ 2147267564, 629510, -865579, 740985, -437242, 184963, -55964, 10687 }, /* 11/4 */
	{ 2147474408, 31341, -50933, 52563, -38409, 21050, -9186, 2906 }, /* 13/4 */
	{ 2147483404, 946, -1775, 2139, -1850, 1231, -694, 286 }, /* 15/4 */
};

/* q(u) = (erf(x)/x - 2/sqrt(pi))/u for u = x^2 <= 1/4 by its Chebyshev fit of degree 4 in signed Q31 (2^-33.5 relative) */
static const int32_t series[5] = { -807725270, 242317490, -57691724, 11185265, -1681780 };

/*
 * The fast erff of the cores without an FPU (PLAN D162, D181), in fixed point and rounded to nearest once without a test or
 * a fallback. Below 1/2, erf(x) = x (2/sqrt(pi) + u q(u)), u = x^2, relative to 2^-31 however small x (subnormal results
 * included). From 1/2 to 3.92, erf directly from the piece of its argument: erf is at least 0.52 there, so an absolute
 * error of 2^-29.9 is below 2^-5.9 of its ulp, and no exponential is needed (erfc from R(x) e^(-x^2), as nofp/speed, would
 * need both). Beyond 3.92, +-1 (C23 F.10.5.1: +-0 itself, +-infinity +-1).
 */
float erff(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	bool negative = (bits >> 31) != 0;
	uint32_t m;
	uint64_t v;
	int32_t p;
	int32_t u;
	int e;

	if (magnitude >= 0x3f000000) {
		const int32_t *a;
		int32_t t;
		int j;

		if (magnitude >= 0x407ae148)
			return magnitude > 0x7f800000 ? x + x : copysignf(1.0f, x);

		/* |x| exactly in units of 2^-28 (below 2^30), the piece j and t = 4 (|x| - c) in Q31, in [-1, 1) */
		e = (int)(magnitude >> 23) - 127;
		u = (int32_t)(((magnitude & __LIBC_FLOAT_FRACTION) | 0x00800000) << (e + 5));
		j = (u - (1 << 27)) >> 27;
		t = (u - (3 << 26) - (j << 27)) * 32;
		a = pieces[j];

		/* The high half of t p is p s with s = t/2 */
		p = a[7];
		for (int k = 6; k >= 0; --k)
			p = a[k] + __libc_fixed_high_signed(t, p);

		/* p in [0.52, 1) in Q31: the 32 bits below its leading one (bit 30); a carry gives 1 */
		return __libc_float_from(__libc_fixed_nearest((uint32_t)p << 2, -1) | (bits & 0x80000000u));
	}

	if (magnitude == 0)
		return x;

	/* |x| = m 2^(e - 31) with bit 31 of m set; u = x^2 in Q32, 0 below 2^-16 (u q(u) is then below 2^-33 relative) */
	m = __libc_fixed_significand(magnitude, &e);
	u = -2 - 2 * e < 32 ? (int32_t)(__libc_fixed_high(m, m) >> (-2 - 2 * e)) : 0;
	p = series[4];
	for (int k = 3; k >= 0; --k)
		p = series[k] + __libc_fixed_high_signed(u, p);

	/* 2/sqrt(pi) in unsigned Q31 plus u q(u), times m: x erf(x)/x in units of 2^(e - 62) */
	v = __libc_fixed_wide(m, 0x906eba82u + (uint32_t)__libc_fixed_high_signed(u, p));

	return __libc_fixed_product_float(v, e - 62, negative);
}
