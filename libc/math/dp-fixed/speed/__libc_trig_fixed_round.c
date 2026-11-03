/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_trig_fixed_round.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "trig-fixed.h"

/*
 * To nearest: the 11 bits below the 53-bit significand decide, unless they
 * lie within error units of the halfway point 0x400; then the true value
 * may lie on the other side, and the caller takes the double-double path
 */
bool __libc_trig_fixed_round(const struct __libc_trig_fixed *value, unsigned int error, double *result)
{
	uint32_t rest = (uint32_t)value->v & 0x7ff;
	uint64_t significand = value->v >> 11;
	int exponent = value->exponent;
	uint64_t bits;

	if (rest - (0x400 - error) <= 2 * error)
		return false;

	if (rest > 0x400) {
		significand += 1;
		if (significand >> 53) {
			significand >>= 1;
			exponent += 1;
		}
	}

	bits = ((uint64_t)value->negative << 63) | ((uint64_t)(exponent + 1023) << 52) | (significand & 0x000fffffffffffffULL);
	__builtin_memcpy(result, &bits, sizeof(*result));
	return true;
}
