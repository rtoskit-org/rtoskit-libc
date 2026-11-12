/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_asin.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * (asin(sqrt(u))/sqrt(u) - 1)/u for u <= 1/4 by its Chebyshev fit of degree 6 in signed Q31 (u p(u) within 2^-33.7,
 * docs/research/libm-fast-float-trig.md, section 19)
 */
static const int32_t series[7] = { 357913942, 161061110, 95880128, 65002480, 50705991, 22708384, 66517319 };

/*
 * asin y = y K(y^2), K(u) = 1 + u p(u) in (1, 1.048] for y <= 1/2. Up to 1/2, y = |x| with u = x^2 in Q32 (0 below 2^-16);
 * above, the half-angle form asin |x| = pi/2 - 2 asin sqrt(z) with z = (1 - |x|)/2, exact: |x| = m 2^-24 makes z = w 2^-25,
 * w = 2^24 - m, and u = z in Q32 is exact too. The square root of z = (2w) 2^-26 from 2w shifted by an even count to
 * [2^30, 2^32).
 */
uint64_t __libc_fixed_asin(uint32_t magnitude, int *exponent, bool *outer)
{
	uint64_t v;
	uint32_t m;
	uint32_t u;
	int32_t up;
	int32_t p;
	int shift;
	int e;
	int z;

	if (magnitude <= 0x3f000000) {
		/* |x| = m 2^(e - 31) with e <= -1; x^2 in Q32 is m^2 2^(2e + 2 - 32) */
		*outer = false;
		m = __libc_fixed_significand(magnitude, &e);
		shift = -2 - 2 * e;
		u = shift < 32 ? __libc_fixed_high(m, m) >> shift : 0;
		e += 1;
	} else {
		*outer = true;
		if (magnitude == 0x3f800000) {
			*exponent = 0;
			return 0;
		}
		u = UINT32_C(0x01000000) - ((magnitude & __LIBC_FLOAT_FRACTION) | 0x00800000);
		shift = (int)stdc_leading_zeros(2 * u) & ~1;

		/* sqrt(z) = sqrt(t) 2^(3 - shift/2), so 2 sqrt(z) = m 2^(-28 - shift/2) with m = sqrt(t) in Q32 */
		m = __libc_fixed_square_root((2 * u) << shift);
		e = 4 - shift / 2;
		u <<= 7;
	}

	p = series[6];
	for (int i = 5; i >= 0; --i)
		p = series[i] + __libc_fixed_high_signed((int32_t)u, p);
	up = (int32_t)(__libc_fixed_wide_signed((int32_t)u, p) >> 31);

	/* m K in units of 2^-31 of m: below 2^64 as K < 1.05 */
	v = ((uint64_t)m << 31) + (__libc_fixed_wide(m, (uint32_t)up) >> 1);
	z = (int)stdc_leading_zeros(v);
	*exponent = e - z;
	return v << z;
}
