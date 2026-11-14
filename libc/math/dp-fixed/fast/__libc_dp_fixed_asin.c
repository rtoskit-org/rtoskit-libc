/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_asin.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/*
 * W = z P(z) in Q64 for z = z 2^-64 <= 1/4, P = (asin sqrt(z) - sqrt(z))/(z sqrt(z)) by fifteen Q64 coefficients fitted here
 * (W within 2^-65.4); one coefficient is negative, so Horner's rule runs in signed 64 bits (every partial sum below 2^62)
 */
static uint64_t series(uint64_t z)
{
	static const int64_t c[15] = {
		0x2aaaaaaaaaaaaaacll,
		0x1333333333332c75ll,
		0x0b6db6db6dbea671ll,
		0x07c71c71c38ff7c1ll,
		0x05ba2e8c7b257769ll,
		0x04713af4c5934576ll,
		0x03933613aa00d90bll,
		0x02f4e04464ddc5eell,
		0x0281d36fbde9697dll,
		0x0214db284f4897b1ll,
		0x024202a9a1a86104ll,
		0x00138dfb6707e15all,
		0x05d1da429c76421all,
		-447356642095981447ll,
		0x07e5d86e86124902ll,
	};
	int64_t p = c[14];

	for (int k = 13; k >= 0; --k)
		p = c[k] + (p < 0 ? -(int64_t)__libc_trig_high64(z, (uint64_t)-p) : (int64_t)__libc_trig_high64(z, (uint64_t)p));

	return __libc_trig_high64(z, (uint64_t)p);
}

/* The double of v 2^(e - 63), v nonzero, normalized by its leading zeros, with sign; rounded once */
static inline double result(uint64_t v, int32_t e, bool negative)
{
	int k = __builtin_clzll(v);

	return __libc_dp_fixed_double(negative ? 0x8000000000000000ull : 0, e - k, v << k);
}

/*
 * The kernel (PLAN D200; docs/research/libm-fast-double.md, section 18): up to 1/2 asin |x| = |x| (1 + W(x^2)) relative,
 * acos x = pi/2 -+ it in Q62; above, z = (1 - |x|)/2 exactly, s = sqrt z, asin s = s (1 + W(z)), asin |x| = pi/2 - 2 asin s
 * and acos x = pi - 2 asin s (x < 0) in Q62, acos x = 2 asin s (x > 0) relative
 */
double __libc_dp_fixed_asin(uint64_t magnitude, bool acos, bool negative)
{
	constexpr uint64_t half_pi = 0x6487ed5110b4611aull;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;
	uint64_t z;
	uint64_t s;
	uint64_t t;
	int32_t h;

	if (magnitude <= 0x3fe0000000000000ull) {
		/* |x| = (m 2^10) 2^(e - 62); z 2^64 = (m 2^11)^2 2^-64 2^(2e + 2), e <= -1 */
		int32_t shift = -(2 * e + 2);
		uint64_t a = m << 10;

		z = shift >= 64 ? 0 : __libc_trig_high64(m << 11, m << 11) >> shift;
		s = a + __libc_trig_high64(a, series(z));
		if (!acos)
			return result(s, e + 1, negative);
		/* asin |x| in Q62: s 2^(e - 62) 2^62 */
		s = -e >= 64 ? 0 : s >> -e;
		return result(negative ? half_pi + s : half_pi - s, 1, false);
	}

	/* 1/2 < |x| < 1: |x| = m 2^-53, z 2^64 = (2^53 - m) 2^10 exactly, normalized by an even shift for the square root */
	z = (0x0020000000000000ull - m) << 10;
	h = __builtin_clzll(z) >> 1;
	s = __libc_dp_fixed_root(z << (2 * h));
	/* asin s = (s/2)(1 + W) 2^(-63 - h), and 2 asin s in Q62 is that >> h */
	t = (s >> 1) + __libc_trig_high64(s >> 1, series(z));
	if (acos && !negative)
		return result(t, 1 - h, false);

	/* pi - 2 asin s for acos (x < 0), pi/2 - 2 asin s for asin, in Q62 */
	return result((acos ? 0xc90fdaa22168c235ull : half_pi) - (t >> h), 1, acos ? false : negative);
}
