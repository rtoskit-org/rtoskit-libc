/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_trig_slow.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* (sin r - r + r^3/6)/r^5 for |r| <= pi/4, from z = r^2: six terms, fitted here (P11.06f; 2^-57.1 relative to sin r) */
static inline double sin_tail(double z)
{
	double p = -0x1.ab17aebd38d7bp-41;

	p = p * z + 0x1.61217ecd5bd75p-33;
	p = p * z - 0x1.ae64541278a2cp-26;
	p = p * z + 0x1.71de3a54606abp-19;
	p = p * z - 0x1.a01a01a019937p-13;
	return p * z + 0x1.1111111111110p-7;
}

/* (cos r - 1 + r^2/2)/r^4 for |r| <= pi/4, from z = r^2: six terms, fitted here (2^-60.5 relative to cos r) */
static inline double cos_tail(double z)
{
	double p = -0x1.8ff9ad8c46497p-37;

	p = p * z + 0x1.1eea7ee54ce92p-29;
	p = p * z - 0x1.27e4f8e453be1p-22;
	p = p * z + 0x1.a01a019de0d99p-16;
	p = p * z - 0x1.6c16c16c15d76p-10;
	return p * z + 0x1.5555555555551p-5;
}

/*
 * The fast sin, cos and tan (and sinf's family from 2^20, __libc_fast_trig_large) from 2^20 and next to a zero, where
 * the common path's three parts of pi/32 are not enough (PLAN D196, D211; docs/research/libm-fast-double.md section
 * 26): x = q pi/2 + r by the strict reduction (__libc_rem_pio2, r as a pair to about 2^-100 relatively), then sin r
 * and cos r directly by the polynomials above, r's low part to first order, so that neither the strict cores nor the
 * common path's table is linked. kind 0 for sin, 1 for cos, 2 for tan; round to nearest, within about 0.6 ulp.
 */
double __libc_fast_trig_slow(double x, int kind)
{
	struct __libc_dd r;
	struct __libc_dd product;
	int q = __libc_rem_pio2(x, &r);
	/*
	 * Near pi/4 the tails are large (r^3/6 is 0.11 of sin r, r^2/2 0.43 of cos r), so their leading terms are exact:
	 * z = r^2 and r^3 as pairs, -r^3/6 to 2^-55 of itself (1/6's rounding) with the product's error, the sum with r by a
	 * fast two-sum; r's low part times cos r_hi to second order; the rest of each tail, below 2^-8 of the result, in
 * plain double
	 */
	struct __libc_dd z = __libc_dd_product(r.hi, r.hi);
	struct __libc_dd cube = __libc_dd_product(r.hi, z.hi);
	struct __libc_dd sixth = __libc_dd_product(cube.hi, -0x1.5555555555555p-3);
	struct __libc_dd sine = __libc_dd_fast_sum(r.hi, sixth.hi);
	double s_hi = sine.hi;
	double s_lo = sine.lo + sixth.lo + r.lo * (1.0 - 0.5 * z.hi) - (cube.lo + r.hi * z.lo) * 0x1.5555555555555p-3 + cube.hi * z.hi * sin_tail(z.hi);
	struct __libc_dd half = __libc_dd_fast_sum(1.0, -0.5 * z.hi);
	double c_hi = half.hi;
	double c_lo = half.lo - 0.5 * z.lo + z.hi * z.hi * cos_tail(z.hi) - r.hi * r.lo;
	double num;
	double num_lo;
	double den;
	double den_lo;
	double quotient;
	bool negative;

	if (kind != 2) {
		q += kind;
		negative = (q & 2) != 0;
		return (q & 1) == 0 ? __libc_fast_signed_sum(s_hi, s_lo, negative) : __libc_fast_signed_sum(c_hi, c_lo, negative);
	}
	/* tan r for an even quadrant, -cot r for an odd one, both pairs normalized; the quotient's remainder to first order */
	product = __libc_dd_fast_sum(s_hi, s_lo);
	num = product.hi;
	num_lo = product.lo;
	product = __libc_dd_fast_sum(c_hi, c_lo);
	den = product.hi;
	den_lo = product.lo;
	if ((q & 1) != 0) {
		double swap = num;
		double swap_lo = num_lo;

		num = -den;
		num_lo = -den_lo;
		den = swap;
		den_lo = swap_lo;
	}
	quotient = num / den;
	product = __libc_dd_product(quotient, den);
	return quotient + (((num - product.hi) - product.lo) + (num_lo - quotient * den_lo)) / den;
}
