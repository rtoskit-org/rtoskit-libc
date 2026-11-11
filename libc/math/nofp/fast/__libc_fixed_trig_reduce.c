/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_trig_reduce.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/* floor(32/pi 2^124), least significant word first, computed here */
static const uint32_t thirty_two_over_pi[4] = { 0xf534ddc0, 0xfc2757d1, 0x4e441529, 0xa2f9836e };

/* |x| = (k + f) pi/32 for 2^-6 <= |x| < 2^20 (fixed-fast.h) */
int __libc_fixed_trig_reduce(uint32_t bits, int64_t *s)
{
	uint32_t m = (bits & 0x007fffff) | 0x00800000;
	int point = 147 - ((int)((bits >> 23) & 0xff) - 127);
	uint32_t product[6];
	uint64_t carry = 0;
	uint64_t f;
	uint64_t magnitude;
	uint32_t low;
	int k;

	for (int i = 0; i < 4; ++i) {
		uint64_t sum = __libc_fixed_wide(m, thirty_two_over_pi[i]) + carry;

		product[i] = (uint32_t)sum;
		carry = sum >> 32;
	}
	product[4] = (uint32_t)carry;
	product[5] = 0;
	k = (int)(__libc_fixed_word_at(product, point) & 63);
	f = ((uint64_t)__libc_fixed_word_at(product, point - 32) << 32) | __libc_fixed_word_at(product, point - 64);
	/* A fraction of 1/2 or more rounds up: f - 1, negative, two's complement as it stands */
	if ((f >> 63) != 0)
		++k;
	magnitude = (int64_t)f < 0 ? -f : f;
	magnitude = __libc_fixed_wide_high((uint32_t)(magnitude >> 32), __LIBC_FIXED_PI_32, &low) + (__libc_fixed_wide_high((uint32_t)magnitude, __LIBC_FIXED_PI_32, &low) >> 32);
	*s = (int64_t)(magnitude >> 19);
	if ((int64_t)f < 0)
		*s = -*s;
	if ((bits >> 31) != 0) {
		k = -k;
		*s = -*s;
	}
	return k;
}
