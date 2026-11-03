/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_trig_fixed_roundf.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "trig-fixed.h"

/*
 * To nearest float (the soft-float cores round to nearest only, D44): the 40
 * bits below the 24-bit significand decide, unless they lie within the
 * kernel's error bound of the halfway point 2^39. A sine or cosine of a float
 * in the kernel's domain is never subnormal, but a result below 2^-126
 * declines all the same.
 */
bool __libc_trig_fixed_roundf(const struct __libc_trig_fixed *value, unsigned int error, float *result)
{
	uint64_t rest = value->v & 0x000000ffffffffffULL;
	uint32_t significand = (uint32_t)(value->v >> 40);
	int exponent = value->exponent;
	uint32_t bits;

	if (exponent < -126 || rest - (0x0000008000000000ULL - error) <= 2 * error)
		return false;

	if (rest > 0x0000008000000000ULL) {
		significand += 1;
		if (significand >> 24) {
			significand >>= 1;
			exponent += 1;
		}
	}

	bits = ((uint32_t)value->negative << 31) | ((uint32_t)(exponent + 127) << 23) | (significand & 0x007fffffu);
	__builtin_memcpy(result, &bits, sizeof(*result));
	return true;
}
