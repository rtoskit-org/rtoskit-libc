/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_log2f_wide.c
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed.h"
#include "libm-fixed.h"

/* The high 64 bits of a b, less than 3 below the truth: the low partial product and the carries out of the cross terms are dropped */
static uint64_t high64(uint64_t a, uint64_t b)
{
	uint32_t ah = (uint32_t)(a >> 32);
	uint32_t al = (uint32_t)a;
	uint32_t bh = (uint32_t)(b >> 32);
	uint32_t bl = (uint32_t)b;

	return __libc_fixed_wide(ah, bh) + (__libc_fixed_wide(ah, bl) >> 32) + (__libc_fixed_wide(al, bh) >> 32);
}

/* a b for signed Q62 numbers of magnitude below 2^62, in Q62 */
static int64_t q62(int64_t a, int64_t b)
{
	uint64_t product = high64(a < 0 ? -(uint64_t)a : (uint64_t)a, b < 0 ? -(uint64_t)b : (uint64_t)b) << 2;

	return (a < 0) != (b < 0) ? -(int64_t)product : (int64_t)product;
}

/*
 * log2 x for powf (PLAN D115, docs/research/libm-fixed.md), to about 2^-44
 * relative: the reduction of __libc_log2f_fixed (x = 2^e m, c' the table's c
 * nearest m, r = m / c' - 1 exact in Q62, |r| < 2^-5.4), then log2(1 + r) =
 * r U(r) / ln 2 with U = log(1 + r) / r = 1 - r/2 + r^2/3 - ... to the term in
 * r^7 (the next is below 2^-46), its coefficients divided by ln 2. The terms
 * from r^3 on are summed by Horner's rule in signed Q31 with r in Q36, as in
 * __libc_log2f_fixed; the last three steps, which the error budget needs
 * wider, in Q62. r U comes from r
 * normalised to 64 bits, so near 1 (e = 0, c' = 1) the result keeps r's
 * relative accuracy; elsewhere |log2 x| >= 0.022 and e + log2 c' + r U / ln 2
 * is summed in units of 2^-55. The result is v 2^(*exponent - 63) with bit 63
 * of v set, or 0 for x = 1.
 */
uint64_t __libc_log2f_wide(uint32_t bits, int *exponent, bool *negative)
{
	int e = (int)(bits >> 23);
	uint32_t m = bits & 0x007fffff;
	uint64_t magnitude;
	uint64_t product;
	int64_t r;
	int64_t u;
	int32_t r36;
	int32_t h;
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

	/* (-1/4 + r/5 - r^2/6 + r^3/7 - r^4/8) / ln 2 in Q31 */
	r36 = (int32_t)((r + (1 << 25)) >> 26);
	h = -387270501;
	h = 442594858 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -516360668 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = 619632802 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -774541002 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);

	/* U / ln 2 = (1 + r (-1/2 + r (1/3 + r h ln 2))) / ln 2 in Q62 */
	u = INT64_C(0x1ec709dc3a03fd75) + ((__libc_fixed_wide_signed(r36, h) + 16) >> 5);
	u = INT64_C(-3326628274461080623) + q62(r, u);
	u = INT64_C(0x5c551d94ae0bf85e) + q62(r, u);

	/* |r| U / ln 2 = product 2^(-61 - shift), U / ln 2 in Q63 */
	magnitude = r < 0 ? -(uint64_t)r : (uint64_t)r;
	shift = (int)stdc_leading_zeros(magnitude);
	product = high64(magnitude << shift, (uint64_t)u << 1);

	if (e == 0 && j == 0) {
		*negative = r < 0;
		shift += 61;
	} else {
		int64_t sum = ((int64_t)e << 55) + __libc_log2f_logarithm[j + 9];
		int64_t term = (int64_t)(product >> (6 + shift));

		sum += r < 0 ? -term : term;
		*negative = sum < 0;
		product = sum < 0 ? -(uint64_t)sum : (uint64_t)sum;
		shift = 55;
	}

	*exponent = 63 - shift - (int)stdc_leading_zeros(product);
	return product << stdc_leading_zeros(product);
}
