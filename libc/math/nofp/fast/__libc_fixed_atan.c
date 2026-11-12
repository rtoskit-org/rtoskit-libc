/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_atan.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * atan(c + s/8) = a0 + a1 s + ... + a5 s^5 for -1/2 <= s < 1/2, c = j/8 + 1/16 the middle of [j/8, (j + 1)/8), j = 1 .. 7:
 * a0 in unsigned Q32, the rest in signed Q31; Chebyshev fits in 256-bit arithmetic, rounded, within 2^-31.3 of atan
 * (docs/research/libm-fast-float-trig.md, section 18)
 */
static const uint32_t constant[7] = { 796063383, 1300880603, 1771289359, 2200695975, 2586804455, 2930527289, 3234760121 };
static const int32_t pieces[7][5] = {
	{ 259318780, -5871330, -1127498, 82192, 7206 }, /* 3/16 */
	{ 244553298, -8702929, -747445, 101467, 606 }, /* 5/16 */
	{ 225309760, -10342070, -352005, 91866, -3962 }, /* 7/16 */
	{ 203915361, -10891620, -31124, 67117, -5497 }, /* 9/16 */
	{ 182279779, -10637023, 182969, 40486, -4931 }, /* 11/16 */
	{ 161692886, -9891809, 299589, 19149, -3558 }, /* 13/16 */
	{ 142867935, -8910689, 344983, 4851, -2201 }, /* 15/16 */
};

/* (atan(q)/q - 1)/u for u = q^2 <= 1/64 by its Chebyshev fit of degree 2 in signed Q31 (2^-32.2 relative) */
static const int32_t series[3] = { -715827855, 429464558, -301270335 };

/*
 * a/b = (n/d) 2^k with the significands' quotient q = n/d in [1/2, 1) in Q32 (__libc_fixed_quotient). Below 1/8, atan(a/b)
 * = (a/b)(1 + u p(u)), u = (a/b)^2 in Q32, relative to 2^-31 however small; from 1/8, a/b in Q32 and the piece of its
 * eighth, absolute to 2^-31.3 where atan is at least 0.124 (1/16 of its ulp). a = b: pi/4.
 */
uint64_t __libc_fixed_atan(uint32_t a, uint32_t b, int *exponent)
{
	int ea;
	int eb;
	uint32_t n = __libc_fixed_significand(a, &ea);
	uint32_t d = __libc_fixed_significand(b, &eb);
	int k = ea - eb;
	const int32_t *c;
	uint64_t v;
	uint32_t q;
	uint32_t x;
	uint32_t u;
	int32_t up;
	int32_t p;
	int32_t t;
	int z;
	int j;

	if (a == b) {
		*exponent = -1;
		return __LIBC_FIXED_PI_62;
	}
	if (n >= d) {
		/* n has 24 significant bits: halving it is exact */
		n >>= 1;
		++k;
	}
	q = __libc_fixed_quotient(n, d);

	if (k <= -3) {
		/* a/b = q 2^(k - 32) below 1/8: u = q^2 2^(2k - 32), 0 below 2^-16 */
		u = -2 * k < 32 ? __libc_fixed_high(q, q) >> (-2 * k) : 0;
		p = series[2];
		p = series[1] + __libc_fixed_high_signed((int32_t)u, p);
		p = series[0] + __libc_fixed_high_signed((int32_t)u, p);
		up = (int32_t)(__libc_fixed_wide_signed((int32_t)u, p) >> 31);

		/* q (1 + u p) in units of 2^(k - 64), u p in (-1/192, 0] */
		v = ((uint64_t)q << 32) - __libc_fixed_wide(q, -(uint32_t)up);
		z = (int)stdc_leading_zeros(v);
		*exponent = k - 1 - z;
		return v << z;
	}

	/* a/b in Q32, in [1/8, 1); t = 16 (a/b - c) in Q31, and the high half of t p is p s */
	x = q >> -k;
	j = (int)(x >> 29);
	t = (int32_t)(x - ((uint32_t)j << 29) - (UINT32_C(1) << 28)) * 8;
	c = pieces[j - 1];
	p = c[4];
	for (int i = 3; i >= 0; --i)
		p = c[i] + __libc_fixed_high_signed(t, p);
	x = constant[j - 1] + (uint32_t)(int32_t)(__libc_fixed_wide_signed(t, p) >> 31);

	z = (int)stdc_leading_zeros(x);
	*exponent = -1 - z;
	return (uint64_t)(x << z) << 32;
}
