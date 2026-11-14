/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_log_core.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* (2^7 |r|)^n/(n + 1) 2^-7n in Q64, n = 1..7: q = sum of (-r)^n/(n + 1), log1p(r) = r + r q (the eighth below 2 units) */
#define B1 0x0100000000000000ull
#define B2 0x0001555555555555ull
#define B3 0x0000020000000000ull
#define B4 0x0000000333333333ull
#define B5 0x0000000005555555ull
#define B6 0x0000000000092492ull
#define B7 0x0000000000001000ull

/*
 * ln z = -log(invc) + log1p(r) with r = z invc - 1: the Q63 by Q31 product exact in 96 bits, less 1, kept in Q64 (|r|
 * <= 2^-7.5); q by Horner's rule in y = 2^7 |r| (unsigned, subtracting for r > 0: every partial value stays positive,
 * the coefficients falling by more than 2^7 each step), r q always negative. The error is a few units of 2^-64: the
 * logarithm's rounding, two floors and the series' tail.
 */
int64_t __libc_dp_fixed_log_core(uint64_t z)
{
	const struct __libc_dp_fixed_log_entry *entry = &__libc_dp_fixed_log_table[(z - 0x5a00000000000000ull) >> 56];
	uint64_t low = __libc_trig_wide((uint32_t)z, entry->invc);
	uint64_t high = __libc_trig_wide((uint32_t)(z >> 32), entry->invc) + (low >> 32);
	/* The product in Q94 less 2^94, in Q64: modulo 2^64, since |r| < 2^63 */
	int64_t r = (int64_t)((high << 2) | ((uint32_t)low >> 30));
	uint64_t a = r < 0 ? -(uint64_t)r : (uint64_t)r;
	uint64_t y = a << 7;
	uint64_t p;

	if (r < 0) {
		p = B6 + __libc_trig_high64(y, B7);
		p = B5 + __libc_trig_high64(y, p);
		p = B4 + __libc_trig_high64(y, p);
		p = B3 + __libc_trig_high64(y, p);
		p = B2 + __libc_trig_high64(y, p);
		p = B1 + __libc_trig_high64(y, p);
	} else {
		p = B6 - __libc_trig_high64(y, B7);
		p = B5 - __libc_trig_high64(y, p);
		p = B4 - __libc_trig_high64(y, p);
		p = B3 - __libc_trig_high64(y, p);
		p = B2 - __libc_trig_high64(y, p);
		p = B1 - __libc_trig_high64(y, p);
	}

	return entry->logc + r - (int64_t)__libc_trig_high64(a, __libc_trig_high64(y, p));
}
