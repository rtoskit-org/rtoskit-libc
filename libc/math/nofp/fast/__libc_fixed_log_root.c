/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_log_root.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * a = m 2^(e - 23). From 2^12, log(2a): the 1/(4a^2) of the root is below 2^-26 of a and the logarithm is above 9 (its ulp
 * 2^-20). Below, S = a^2 +- 1 = m^2 +- 2^(46 - 2e) exactly in units of 2^(2e - 46) (below 2^49), shifted by an even count so
 * that its top 32 bits t are in [2^30, 2^32): sqrt(S) = sqrt(t) 2^(-shift/2) in those units, so a + sqrt(a^2 +- 1) =
 * (m 2^32 + sqrt(t) 2^(32 - shift/2)) 2^(e - 55), and its logarithm (at least 0.48) by __libc_fixed_log.
 */
uint64_t __libc_fixed_log_root(uint32_t magnitude, bool minus, int *exponent)
{
	int e = (int)(magnitude >> 23) - 127;
	uint32_t m = (magnitude & __LIBC_FLOAT_FRACTION) | 0x00800000;
	uint64_t square;
	uint64_t sum;
	uint32_t r;
	bool negative;
	int shift;
	int k;

	if (e >= 12)
		return __libc_fixed_log((uint64_t)m << 40, e + 1, exponent, &negative);

	square = __libc_fixed_wide(m, m);
	square = minus ? square - (UINT64_C(1) << (46 - 2 * e)) : square + (UINT64_C(1) << (46 - 2 * e));
	shift = (int)stdc_leading_zeros(square) & ~1;
	r = __libc_fixed_square_root((uint32_t)((square << shift) >> 32));

	sum = ((uint64_t)m << 32) + ((uint64_t)r << (32 - shift / 2));
	k = (int)stdc_leading_zeros(sum);
	return __libc_fixed_log(sum << k, e + 8 - k, exponent, &negative);
}
