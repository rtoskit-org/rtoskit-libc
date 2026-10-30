/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_softdouble_d2f.c
 *
 * Created on: Oct 22, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "internal/softdouble.h"

/*
 * (float)a, rounded to nearest (C23 6.3.1.5p2; PLAN D99), for any a: the
 * top 24 bits of the significand with a guard bit and a sticky bit,
 * rounded once, on the subnormal grid if the result is below float's
 * normal range. A NaN keeps the top of its payload, quietened.
 * __aeabi_d2f's fast path, for a result in the normal range, is Thumb-2
 * assembly (__aeabi_d2f.S); everything else comes here.
 */
__LIBC_AEABI float __libc_softdouble_d2f(double a)
{
	uint64_t bits = __libc_softdouble_bits(a);
	uint32_t high = (uint32_t)(bits >> 32);
	uint32_t low = (uint32_t)bits;
	uint32_t sign = high & 0x80000000u;
	uint32_t field = (high >> 20) & 0x7ff;
	uint64_t significand;
	uint32_t x;
	uint32_t result;
	int exponent;

	/*
	 * A result in float's normal range: the top 24 bits of the significand,
	 * rounded on the 29 below them, the first being half an ulp. A carry
	 * moves into the exponent, and from the largest finite float to
	 * infinity.
	 */
	if (field - (1023 - 126) < 254) {
		result = sign | ((field - (1023 - 127)) << 23 | (high & 0x000fffffu) << 3 | low >> 29);
		if ((low & 0x10000000u) != 0 && (low & 0x2fffffffu) != 0)
			++result;
		return __libc_softdouble_float_from(result);
	}

	if (field == 0x7ff) {
		if (((high & 0x000fffffu) | low) == 0)
			return __libc_softdouble_float_from(sign | 0x7f800000u);
		/* The top 23 bits of the fraction only: above them lie the exponent's ones, which would set the sign */
		return __libc_softdouble_float_from(sign | 0x7fc00000u | ((uint32_t)(bits >> 29) & 0x007fffffu));
	}
	if (field > 1023)
		return __libc_softdouble_float_from(sign | 0x7f800000u);
	if ((bits << 1) == 0)
		return __libc_softdouble_float_from(sign);

	/*
	 * Below float's normal range: x holds 24 bits above a guard bit and a
	 * sticky bit, shifted to the subnormal grid first so that it is rounded
	 * once; the value is x 2^(exponent - 152).
	 */
	significand = __libc_softdouble_unpack(bits, &exponent);
	exponent -= 1023 - 127;
	x = (uint32_t)(significand >> 27) | ((significand & ((1u << 27) - 1)) != 0);
	{
		unsigned int shift = (unsigned int)(1 - exponent);

		x = shift >= 26 ? x != 0 : (x >> shift) | ((x & ((1u << shift) - 1)) != 0);
	}
	result = x >> 2;
	if ((x & 2) != 0 && (x & 5) != 0)
		++result;

	return __libc_softdouble_float_from(sign | result);
}
