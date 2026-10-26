/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_log2f_fixed.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed.h"
#include "libm-fixed.h"

/*
 * log2 x = e + log2 c' + log2(1 + r) for a positive finite x = 2^e m, m in
 * [sqrt(1/2), sqrt(2)) as an unsigned Q31 number, c' the table's c nearest
 * m and r = m / c' - 1, exact as a Q62 product less 1, |r| < 2^-5.4.
 * log2(1 + r) = r Q(r), Q the series (1 - r/2 + r^2/3 - ...) / ln 2 to the
 * term in r^5 (the next is below 2^-35 of Q), by Horner's rule in signed
 * Q31 with r in Q36, each step rounded to nearest, and the first coefficient unsigned. r Q comes from r
 * normalised to 32 bits, so near 1 (e = 0, c' = 1) the result keeps r's
 * relative accuracy; elsewhere |log2 x| >= 0.022 and the sum is formed in
 * units of 2^-55. The result is v 2^(*exponent - 63) with bit 63 of v set,
 * or 0 for x = 1; the error of its top 32 bits after the leading one,
 * measured over every float, is below __LIBC_FIXED_LOG2_ERROR units.
 */
uint64_t __libc_log2f_fixed(uint32_t bits, int *exponent, bool *negative)
{
	int e = (int)(bits >> 23);
	uint32_t m = bits & 0x007fffff;
	uint64_t magnitude;
	uint64_t product;
	int64_t r;
	int32_t r36;
	int32_t h;
	uint32_t q;
	int scale;
	int shift;
	int j;

	/* A subnormal x is normalised */
	if (e == 0) {
		shift = (int)stdc_leading_zeros(m) - 8;
		m <<= shift;
		e = 1 - shift;
	} else {
		m |= 0x00800000;
	}
	e -= 127;
	if (m > 0x00b504f3) {
		m <<= 7;
		++e;
	} else {
		m <<= 8;
	}

	/* j = round(32 (m - 1)), in -9 .. 13 */
	j = ((int32_t)(m - 0x80000000u) + (1 << 25)) >> 26;
	r = (int64_t)(__libc_fixed_wide(m, __libc_log2f_inverse[j + 9]) - (UINT64_C(1) << 62));
	if (r == 0 && e == 0 && j == 0)
		return 0;

	r36 = (int32_t)((r + (1 << 25)) >> 26);
	h = -516360668;
	h = 619632802 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -774541002 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = 1032721336 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -1549082005 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	q = 0xb8aa3b29u + (uint32_t)((__libc_fixed_high_signed(r36, h) + 8) >> 4);

	/* |r| Q = product 2^(-61 - shift) */
	magnitude = r < 0 ? -(uint64_t)r : (uint64_t)r;
	shift = magnitude != 0 ? (int)stdc_leading_zeros(magnitude) : 0;
	product = __libc_fixed_wide((uint32_t)((magnitude << shift) >> 32), q);

	if (e == 0 && j == 0) {
		*negative = r < 0;
		scale = -61 - shift;
	} else {
		int64_t sum = ((int64_t)e << 55) + __libc_log2f_logarithm[j + 9];
		int64_t term = 6 + shift < 64 ? (int64_t)(product >> (6 + shift)) : 0;

		sum += r < 0 ? -term : term;
		*negative = sum < 0;
		product = sum < 0 ? -(uint64_t)sum : (uint64_t)sum;
		scale = -55;
	}

	shift = (int)stdc_leading_zeros(product);
	*exponent = 63 + scale - shift;
	return product << shift;
}
