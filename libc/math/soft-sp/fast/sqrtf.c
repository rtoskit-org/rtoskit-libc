/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sqrtf.c
 *
 * Created on: Nov 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast sqrtf of armv7-m (PLAN D162, D242; docs/research/libm-fast-double.md section 38), still correctly rounded
 * (C23 F.3, D222), in place of nofp/fast's digit by digit (Armv6-M's choice, where a 32 x 32 -> 64 product is a call):
 * x = m 2^E with m of 24 bits (subnormals normalised), t = m 2^-25 or m 2^-24 in [1/4, 1) by E's parity, sqrt(t) in
 * Q32 by __libc_fixed_square_root (within a unit of 2^-32, UMULL inline), its top 24 bits q, and the rounding from the
 * 8 bits below, except within 2 units of the midpoint, where (2q + 1)^2 against the radicand decides exactly (never
 * equal: the square is odd).
 */
float sqrtf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t m = bits & 0x007fffffu;
	int32_t e = (int32_t)(bits >> 23);
	uint32_t w;
	uint32_t s;
	uint32_t q;
	uint32_t low;

	/* +-0, +infinity and NaN: x itself (a NaN quietened); a negative number is invalid */
	if ((bits << 1) == 0 || bits >= 0x7f800000u)
		return bits > 0x80000000u && bits <= 0xff800000u ? __libc_math_invalidf() : x + x;
	if (e == 0) {
		int shift = (int)stdc_leading_zeros(m) - 8;

		m <<= shift;
		e = 1 - shift;
	} else {
		m |= 0x00800000u;
	}
	/* E = e - 127: t = m 2^-25 (E even, e odd) or m 2^-24 (E odd), and sqrt(x) = sqrt(t) 2^((E + 2) >> 1) */
	w = m << ((e & 1) == 0 ? 8 : 7);
	s = __libc_fixed_square_root(w);
	q = s >> 8;
	low = s & 0xffu;
	if (low - 0x7eu <= 4u) {
		uint64_t odd = 2 * (uint64_t)q + 1;

		q += odd * odd < ((uint64_t)w << 18);
	} else {
		q += low > 0x80u;
	}

	return __libc_float_from((uint32_t)((((e - 127 + 2) >> 1) + 125) << 23) + q);
}
