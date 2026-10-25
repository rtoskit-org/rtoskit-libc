/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fdiv.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "softfloat.h"

/*
 * a / b in integers (IEEE 754-2019 5.4.1, PLAN D85): 26 quotient bits by
 * restoring division of the significands, the remainder as the sticky bit,
 * then one rounding.
 */
float __aeabi_fdiv(float a, float b)
{
	uint32_t ua = __libc_softfloat_bits(a);
	uint32_t ub = __libc_softfloat_bits(b);
	uint32_t sign = (ua ^ ub) & __LIBC_SOFTFLOAT_SIGN;
	uint32_t ma;
	uint32_t mb;
	uint32_t quotient = 0;
	int ea;
	int eb;

	if ((ua << 1) > (__LIBC_SOFTFLOAT_INFINITY << 1))
		return __libc_softfloat_from(ua | __LIBC_SOFTFLOAT_QUIET);
	if ((ub << 1) > (__LIBC_SOFTFLOAT_INFINITY << 1))
		return __libc_softfloat_from(ub | __LIBC_SOFTFLOAT_QUIET);
	if ((ua << 1) == (__LIBC_SOFTFLOAT_INFINITY << 1)) {
		/* Infinity over infinity is invalid (7.2) */
		if ((ub << 1) == (__LIBC_SOFTFLOAT_INFINITY << 1))
			return __libc_softfloat_from(__LIBC_SOFTFLOAT_DEFAULT_NAN);
		return __libc_softfloat_from(sign | __LIBC_SOFTFLOAT_INFINITY);
	}
	if ((ub << 1) == (__LIBC_SOFTFLOAT_INFINITY << 1))
		return __libc_softfloat_from(sign);
	if ((ub << 1) == 0) {
		/* Zero over zero is invalid (7.2); anything else over zero is an infinity (7.3) */
		if ((ua << 1) == 0)
			return __libc_softfloat_from(__LIBC_SOFTFLOAT_DEFAULT_NAN);
		return __libc_softfloat_from(sign | __LIBC_SOFTFLOAT_INFINITY);
	}
	if ((ua << 1) == 0)
		return __libc_softfloat_from(sign);

	ma = __libc_softfloat_unpack(ua, &ea);
	mb = __libc_softfloat_unpack(ub, &eb);

	/* A quotient in [1, 2): ma 2^-25 / mb gives 26 bits from 2^25 */
	int exponent = ea - eb + 127;

	if (ma < mb) {
		ma <<= 1;
		--exponent;
	}
	for (int i = 0; i < 26; ++i) {
		quotient <<= 1;
		if (ma >= mb) {
			ma -= mb;
			quotient |= 1;
		}
		ma <<= 1;
	}

	/* The last quotient bit and the remainder are both below the guard: together they are the sticky bit */
	return __libc_softfloat_from(__libc_softfloat_pack(sign, exponent, quotient | (ma != 0)));
}
