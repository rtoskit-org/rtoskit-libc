/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_tan_fixed.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "trig-fixed.h"

/*
 * 2^127 / d for a significand d with bit 63 set, to about 64 bits, in
 * (2^63, 2^64): a 17-bit seed from one 32 / 16-bit division (UDIV on the
 * Thumb-2 cores, __aeabi_uidiv on Armv6-M), then Newton's iteration y (2 -
 * d y 2^-127) three times (17, 34, 64 bits), each two high products. The
 * exact reciprocal of 2^63, 2^64, saturates to 2^64 - 1.
 */
static uint64_t reciprocal(uint64_t d)
{
	uint64_t y = (uint64_t)(0xffffffffu / (uint32_t)(d >> 48)) << 47;

	for (int i = 0; i < 3; ++i) {
		/* d y 2^-64 is about 2^63; 2^64 minus it is the factor 2 - d y 2^-127 in units of 2^-63 */
		uint64_t factor = 0 - __libc_trig_high64(d, y);
		uint64_t next = __libc_trig_high64(y, factor);

		y = (next >> 63) != 0 ? UINT64_MAX : next << 1;
	}
	return y;
}

/*
 * tan x = sin x / cos x from one kernel call, in every quadrant: s / c =
 * (s.v / c.v) 2^(s.exponent - c.exponent), and s.v 2^127 / c.v 2^-64 =
 * (s.v / c.v) 2^63, a quotient in (2^62, 2^64)
 */
bool __libc_tan_fixed(double x, struct __libc_trig_fixed *tangent)
{
	struct __libc_trig_fixed sine;
	struct __libc_trig_fixed cosine;
	uint64_t q;
	int exponent;

	if (!__libc_trig_fixed(x, __LIBC_TRIG_FIXED_SIN | __LIBC_TRIG_FIXED_COS, &sine, &cosine))
		return false;

	q = __libc_trig_high64(sine.v, reciprocal(cosine.v));
	exponent = sine.exponent - cosine.exponent;
	if ((q >> 63) == 0) {
		q <<= 1;
		exponent -= 1;
	}
	*tangent = (struct __libc_trig_fixed){ .v = q, .exponent = exponent, .negative = sine.negative != cosine.negative };
	return true;
}
