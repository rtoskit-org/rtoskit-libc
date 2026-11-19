/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * hypotf.c
 *
 * Created on: Nov 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast hypotf of the cores without an FPU (PLAN D162, D222, D242; docs/research/libm-fast-double.md section 38),
 * correctly rounded in integers. With a >= b and their 24-bit significands, beyond an exponent gap of 13 b^2/a^2 is
 * below 2^-26 and a is the result (inexact). Otherwise S = a^2 + b^2 is formed in 64 bits, exact up to a gap of 7 and
 * with a sticky bit beyond, normalised to [2^62, 2^64); its top 32 bits give sqrt in Q32 (__libc_fixed_square_root,
 * within two units of 2^-32 of the root of S), and the rounding comes from the 8 bits below the result's 24, except
 * within 3 units of the midpoint, where (2q + 1)^2 against S decides exactly; a tie (a Pythagorean triple) rounds to
 * even.
 */
float hypotf(float x, float y)
{
	uint32_t ax = __libc_float_bits(x) & 0x7fffffffu;
	uint32_t ay = __libc_float_bits(y) & 0x7fffffffu;
	uint32_t ma;
	uint32_t mb;
	uint32_t q;
	uint32_t rest;
	uint32_t half;
	uint32_t bits;
	int32_t field;
	int drop;
	uint32_t s;
	uint64_t sum;
	uint64_t odd;
	uint64_t square;
	int32_t ea;
	int32_t eb;
	int32_t gap;
	int shift;
	bool sticky = false;

	/* An infinity wins over a NaN (F.10.4.3) */
	if (ax >= 0x7f800000u || ay >= 0x7f800000u) {
		if (ax == 0x7f800000u || ay == 0x7f800000u)
			return INFINITY;
		return x + y;
	}
	if (ax < ay) {
		uint32_t swap = ax;

		ax = ay;
		ay = swap;
	}
	if (ay == 0)
		return __libc_float_from(ax);
	ea = (int32_t)(ax >> 23);
	eb = (int32_t)(ay >> 23);
	ma = ax & 0x007fffffu;
	mb = ay & 0x007fffffu;
	if (ea == 0) {
		shift = (int)stdc_leading_zeros(ma) - 8;
		ma <<= shift;
		ea = 1 - shift;
	} else {
		ma |= 0x00800000u;
	}
	if (eb == 0) {
		shift = (int)stdc_leading_zeros(mb) - 8;
		mb <<= shift;
		eb = 1 - shift;
	} else {
		mb |= 0x00800000u;
	}
	gap = ea - eb;
	/* b/a < 2^-13: a (1 + b^2/(2 a^2)) rounds to a (inexact is unspecified in fast, fast-math.md section 4.1) */
	if (gap > 13)
		return __libc_float_from(ax);
	/* S = (ma^2 + mb^2 2^(-2 gap)) 2^14, below 2^63 */
	sum = __libc_fixed_wide(ma, ma) << 14;
	square = __libc_fixed_wide(mb, mb);
	if (2 * gap <= 14) {
		sum += square << (14 - 2 * gap);
	} else {
		sticky = (square & ((UINT64_C(1) << (2 * gap - 14)) - 1)) != 0;
		sum += square >> (2 * gap - 14);
	}
	/* Normalised to [2^62, 2^64) by an even shift, so that the root's exponent is exact */
	shift = (int)stdc_leading_zeros(sum) & ~1;
	sum <<= shift;
	s = __libc_fixed_square_root((uint32_t)(sum >> 32));
	/*
	 * hypot^2 = S 2^(-14 - shift) 2^(2 (ea - 150)) with S normalised, so hypot = sqrt(S 2^-64) 2^(ea - 125 - shift/2): q
	 * 2^(ea - 149 - shift/2) with q of 24 bits, exponent field ea + 1 - shift/2. Below the normal range the significand
	 * loses 1 - field more bits (drop up to 32: hypot >= a >= 2^-149)
	 */
	field = ea + 1 - shift / 2;
	drop = field < 1 ? 9 - field : 8;
	q = (uint32_t)((uint64_t)s >> drop);
	rest = (uint32_t)(s & ((UINT64_C(1) << drop) - 1));
	half = UINT32_C(1) << (drop - 1);
	if (rest - half + 3u <= 6u) {
		/* sqrt(S 2^-64) against (2q + 1) 2^(drop - 33): S against (2q + 1)^2 2^(2 drop - 2) */
		odd = 2 * (uint64_t)q + 1;
		square = (odd * odd) << (2 * drop - 2);
		if (square < sum || (square == sum && (sticky || (q & 1) != 0)))
			q++;
	} else {
		q += rest > half;
	}
	if (field < 1) {
		/* A subnormal result (q at most 2^23, the carry making the smallest normal), underflow when inexact */
		if (sticky || (((uint64_t)q * q) << (2 * drop)) != sum) {
			volatile float tiny = 0x1p-100f;

			tiny *= tiny;
		}
		return __libc_float_from(q);
	}
	bits = ((uint32_t)(field - 1) << 23) + q;
	if (bits >= 0x7f800000u) {
		/* Overflow, raised by a product at run time (the fast files drop -frounding-math, so a constant would fold) */
		volatile float huge = 0x1p127f;

		return huge * huge;
	}
	return __libc_float_from(bits);
}
