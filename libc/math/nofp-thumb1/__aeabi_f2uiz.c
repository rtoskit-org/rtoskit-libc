/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_f2uiz.c
 *
 * Created on: Oct 22, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "softfloat.h"

/*
 * (unsigned int)a, truncated toward zero (C23 6.3.1.4p1, RTABI 4.1.2; PLAN
 * D102): the 24-bit significand shifted by the unbiased exponent less 23.
 * Out of range the behaviour is undefined; the result saturates, and a
 * negative value and NaN give 0. libgcc's version subtracts in float, which
 * brings its own subtraction from inside libgcc in place of this layer's.
 */
__LIBC_AEABI unsigned int __aeabi_f2uiz(float a)
{
	uint32_t bits = __libc_softfloat_bits(a);
	int field = (int)((bits >> 23) & 0xff);
	uint32_t significand = (bits & 0x007fffffu) | 0x00800000u;

	if ((int32_t)bits < 0 || field < 127 || (bits << 1) > (__LIBC_SOFTFLOAT_INFINITY << 1))
		return 0;
	if (field >= 127 + 32)
		return UINT32_MAX;
	if (field >= 150)
		return significand << (field - 150);

	return significand >> (150 - field);
}
