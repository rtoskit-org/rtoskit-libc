/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_log_near.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbool.h>
#include <stdint.h>

#include "dp-fixed-fast.h"

/* (2^5 |s|)^n/(n + 1) 2^-5n in Q64, n = 1..11 (the twelfth is below a unit) */
#define A1 0x0400000000000000ull
#define A2 0x0015555555555555ull
#define A3 0x0000800000000000ull
#define A4 0x0000033333333333ull
#define A5 0x0000001555555555ull
#define A6 0x0000000092492492ull
#define A7 0x0000000004000000ull
#define A8 0x00000000001c71c7ull
#define A9 0x000000000000cccdull
#define A10 0x00000000000005d1ull
#define A11 0x000000000000002bull

/*
 * log1p(s) = s (1 + q) for |s| < 2^-5, q = sum of (-s)^n/(n + 1) in Q64 from y = 2^5 |s| by Horner's rule (unsigned,
 * subtracting for s > 0; the six innermost steps' operands fit in 32 bits), then s (1 + q) from the normalized s:
 * relative however small s is
 */
uint64_t __libc_dp_fixed_log_near(uint64_t s, int32_t *e, bool negative)
{
	int32_t shift = -*e - 6;
	uint64_t y = shift < 64 ? s >> shift : 0;
	uint64_t h;
	uint64_t p;
	uint64_t w;

	if (negative) {
		p = A10 + __libc_trig_high64(y, A11);
		p = A9 + __libc_trig_high64(y, p);
		p = A8 + __libc_trig_high64(y, p);
		p = A7 + __libc_trig_high64(y, p);
		p = A6 + __libc_trig_high64(y, p);
		p = A5 + __libc_trig_high64(y, p);
		p = A4 + __libc_trig_high64(y, p);
		p = A3 + __libc_trig_high64(y, p);
		p = A2 + __libc_trig_high64(y, p);
		p = A1 + __libc_trig_high64(y, p);
		w = 0x8000000000000000ull + (__libc_trig_high64(y, p) >> 1);
	} else {
		p = A10 - __libc_trig_high64(y, A11);
		p = A9 - __libc_trig_high64(y, p);
		p = A8 - __libc_trig_high64(y, p);
		p = A7 - __libc_trig_high64(y, p);
		p = A6 - __libc_trig_high64(y, p);
		p = A5 - __libc_trig_high64(y, p);
		p = A4 - __libc_trig_high64(y, p);
		p = A3 - __libc_trig_high64(y, p);
		p = A2 - __libc_trig_high64(y, p);
		p = A1 - __libc_trig_high64(y, p);
		w = 0x8000000000000000ull - (__libc_trig_high64(y, p) >> 1);
	}

	/* s 2^(e - 63) w 2^-63 = h 2^(e - 62), h within 2^-6 of 2^62 or 2^63 */
	h = __libc_trig_high64(s, w);
	if ((h >> 63) != 0) {
		*e += 1;
		return h;
	}
	if ((h >> 62) != 0)
		return h << 1;
	*e -= 1;

	return h << 2;
}
