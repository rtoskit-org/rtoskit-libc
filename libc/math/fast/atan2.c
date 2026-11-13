/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast atan2 of the double-FPU cores (PLAN D162, D198; docs/research/libm-fast-double.md, section 16): for normal
 * coordinates from 2^-959 to 2^960 whose exponents differ by at most 60, the smaller magnitude over the larger with its
 * exact remainder, the table kernel, the quadrant's offset (0, pi/2 or pi) as a pair and y's sign; one rounding. Every
 * other case (C23 7.12.4.4, F.10.1.4's zeros, infinities and NaN, and the far exponents) is the strict file's code.
 */
double atan2(double y, double x)
{
	double ax = __builtin_fabs(x);
	double ay = __builtin_fabs(y);
	uint32_t ex = (uint32_t)(__libc_double_bits(ax) >> 52);
	uint32_t ey = (uint32_t)(__libc_double_bits(ay) >> 52);
	struct __libc_dd offset;
	struct __libc_dd sum;
	double q;
	double q_lo;
	double k;
	double k_lo;
	bool subtract;

	/* Both exponents in [64, 1983) and at most 60 apart, as unsigned differences (a NaN or infinity has 2047) */
	if (ex - 64 >= 1919 || ey - 64 >= 1919 || ex - ey + 60 > 120)
		return __libc_fast_atan2_slow(y, x);

	if (ay <= ax) {
		q = __libc_fast_quotient(ay, ax, &q_lo);
		offset = signbit(x) ? __LIBC_PI : (struct __libc_dd){ .hi = 0.0, .lo = 0.0 };
		subtract = signbit(x) != 0;
	} else {
		q = __libc_fast_quotient(ax, ay, &q_lo);
		offset = __LIBC_PI_2;
		subtract = signbit(x) == 0;
	}
	k = __libc_fast_atan_kernel(q, q_lo, &k_lo);
	if (subtract) {
		k = -k;
		k_lo = -k_lo;
	}

	/* offset + k: the offset is 0 or at least pi/2 > |k| */
	sum = __libc_dd_fast_sum(offset.hi, k);

	return __libc_fast_signed_sum(sum.hi, (sum.lo + offset.lo) + k_lo, signbit(y) != 0);
}
