/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fmodf.c
 *
 * Created on: Nov 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast fmodf of armv7-m (PLAN D162, D242; docs/research/libm-fast-double.md section 38), exact as
 * C23 7.12.10.1 asks (on Armv6-M the remainder of the soft doubles is level with picolibc and smaller, so it stays): on
 * the 24-bit significands in integers, one bit of the exponent difference a step (r < 2 m_y below
 * 2^25), in place of the remainder of the soft doubles; the result normalised, subnormal where it falls below 2^-126.
 */
float fmodf(float x, float y)
{
	uint32_t xbits = __libc_float_bits(x);
	uint32_t ybits = __libc_float_bits(y);
	uint32_t sign = xbits & 0x80000000u;
	uint32_t ax = xbits & 0x7fffffffu;
	uint32_t ay = ybits & 0x7fffffffu;
	uint32_t mx;
	uint32_t my;
	int32_t ex;
	int32_t ey;
	int shift;

	/* NaN, x infinite or y zero: x + y for a NaN, else invalid (F.10.7.1) */
	if (ax >= 0x7f800000u || ay > 0x7f800000u || ay == 0)
		return ax > 0x7f800000u || ay > 0x7f800000u ? x + y : __libc_math_invalidf();
	if (ax < ay)
		return x;
	if (ax == ay)
		return __libc_float_from(sign);
	ex = (int32_t)(ax >> 23);
	ey = (int32_t)(ay >> 23);
	mx = ax & 0x007fffffu;
	my = ay & 0x007fffffu;
	if (ex == 0) {
		shift = (int)stdc_leading_zeros(mx) - 8;
		mx <<= shift;
		ex = 1 - shift;
	} else {
		mx |= 0x00800000u;
	}
	if (ey == 0) {
		shift = (int)stdc_leading_zeros(my) - 8;
		my <<= shift;
		ey = 1 - shift;
	} else {
		my |= 0x00800000u;
	}
	/* mx 2^ex mod my 2^ey: r takes one bit of the difference a step */
	for (int32_t d = ex - ey; d > 0; d--) {
		if (mx >= my)
			mx -= my;
		mx <<= 1;
	}
	if (mx >= my)
		mx -= my;
	if (mx == 0)
		return __libc_float_from(sign);
	shift = (int)stdc_leading_zeros(mx) - 8;
	mx <<= shift;
	ey -= shift;
	/* Below the normal range the result is exact on the subnormal grid (it is a multiple of y's unit) */
	if (ey < 1)
		return __libc_float_from(sign | (mx >> (1 - ey)));
	return __libc_float_from(sign | (((uint32_t)(ey - 1) << 23) + mx));
}
