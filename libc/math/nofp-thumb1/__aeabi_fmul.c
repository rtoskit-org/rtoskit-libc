/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fmul.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "softfloat.h"

/*
 * a * b in integers (IEEE 754-2019 5.4.1, PLAN D85): the 48-bit product
 * of the 24-bit significands from four products of 12-bit halves, each
 * within 32 bits (Armv6-M multiplies 32 by 32 to 32), then one rounding.
 */
float __aeabi_fmul(float a, float b)
{
	uint32_t ua = __libc_softfloat_bits(a);
	uint32_t ub = __libc_softfloat_bits(b);
	uint32_t sign = (ua ^ ub) & __LIBC_SOFTFLOAT_SIGN;
	uint32_t ma;
	uint32_t mb;
	uint32_t low;
	uint32_t middle;
	uint32_t high;
	uint32_t x;
	int ea;
	int eb;

	if ((ua << 1) > (__LIBC_SOFTFLOAT_INFINITY << 1))
		return __libc_softfloat_from(ua | __LIBC_SOFTFLOAT_QUIET);
	if ((ub << 1) > (__LIBC_SOFTFLOAT_INFINITY << 1))
		return __libc_softfloat_from(ub | __LIBC_SOFTFLOAT_QUIET);
	if ((ua << 1) == (__LIBC_SOFTFLOAT_INFINITY << 1) || (ub << 1) == (__LIBC_SOFTFLOAT_INFINITY << 1)) {
		/* Infinity times zero is invalid (7.2) */
		if ((ua << 1) == 0 || (ub << 1) == 0)
			return __libc_softfloat_from(__LIBC_SOFTFLOAT_DEFAULT_NAN);
		return __libc_softfloat_from(sign | __LIBC_SOFTFLOAT_INFINITY);
	}
	if ((ua << 1) == 0 || (ub << 1) == 0)
		return __libc_softfloat_from(sign);

	ma = __libc_softfloat_unpack(ua, &ea);
	mb = __libc_softfloat_unpack(ub, &eb);

	/* ma mb = high 2^24 + low, from the halves of 12 bits: every partial sum stays below 2^32 */
	{
		uint32_t a1 = ma >> 12;
		uint32_t a0 = ma & 0xfff;
		uint32_t b1 = mb >> 12;
		uint32_t b0 = mb & 0xfff;
		uint32_t product = a0 * b0;

		middle = a1 * b0 + a0 * b1 + (product >> 12);
		low = ((middle & 0xfff) << 12) | (product & 0xfff);
		high = a1 * b1 + (middle >> 12);
	}

	/* The product lies in [2^46, 2^48): take 24 bits, the next as the guard, and the rest as sticky */
	ea += eb - 126;
	if (high < (1u << 23)) {
		high = (high << 1) | (low >> 23);
		low <<= 1;
		--ea;
	}
	x = (high << 2) | ((low >> 22) & 2) | ((low & 0x007fffffu) != 0);

	return __libc_softfloat_from(__libc_softfloat_pack(sign, ea, x));
}
