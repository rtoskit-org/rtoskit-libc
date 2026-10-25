/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_d2uiz.c
 *
 * Created on: Oct 22, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "internal/softdouble.h"

/*
 * (unsigned int)a, truncated toward zero (C23 6.3.1.4p1, RTABI 4.1.2; PLAN
 * D102), in 32-bit words: the top 32 bits of the significand shifted right
 * 31 - t places for an unbiased exponent t in [0, 31]. Out of range the
 * behaviour is undefined; the result saturates, and a negative value and
 * NaN give 0. libgcc's version subtracts in double, which brings its own
 * subtraction from inside libgcc in place of this layer's.
 */
__LIBC_AEABI unsigned int __aeabi_d2uiz(double a)
{
	uint32_t high = (uint32_t)(__libc_softdouble_bits(a) >> 32);
	uint32_t low = (uint32_t)__libc_softdouble_bits(a);
	uint32_t shift = 1023 + 31 - ((high >> 20) & 0x7ff);

	if ((int32_t)high < 0)
		return 0;
	if (shift >= 32) {
		/* Below 1, or 2^32 and above; NaN gives 0 */
		if ((int32_t)shift > 0)
			return 0;
		if ((high << 1) > 0xffe00000u || ((high << 1) == 0xffe00000u && low != 0))
			return 0;
		return UINT32_MAX;
	}

	return ((high << 11) | (low >> 21) | 0x80000000u) >> shift;
}
