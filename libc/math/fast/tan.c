/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast tan of the double-FPU cores (PLAN D162, D196; docs/research/libm-fast-double.md, section 14): sin and cos of
 * a = j pi/32 + t as pairs from sin's kernel, then their quotient with its remainder (the product's error exact): tan a
 * for an even quadrant, -cot a for an odd one (the period is pi). The divisor is at least sin(pi/64) away from the slow
 * path, so the quotient keeps the pairs' accuracy. C23 7.12.4.7, F.10.1.7: an infinity is a domain error; +-0 returns
 * itself.
 */
double tan(double x)
{
	double a = __builtin_fabs(x);
	struct __libc_dd product;
	uint32_t n;
	double t;
	double t_lo;
	double z;
	double sp;
	double cp;
	double num;
	double num_lo;
	double den;
	double den_lo;
	double q;
	double r;

	if (isless(a, 0x1p-27))
		return __libc_math_tiny(x, x);
	if (!isless(a, 0x1p20)) {
		if (isnan(x))
			return x + x;
		if (isinf(x))
			return __libc_math_invalid();
		return __libc_fast_trig_slow(x, 2);
	}

	n = __libc_fast_rem_pio32(a, &t, &t_lo);
	if (__libc_fast_trig_near_zero(n, t))
		return __libc_fast_trig_slow(x, 2);

	/* sin a and cos a without the quadrant's sign (n mod 16 picks a), then tan or -cot by bit 4 of n */
	z = t * t;
	sp = __libc_fast_sin_poly(z);
	cp = __libc_fast_cos_poly(z);
	num = __libc_fast_sin_turn(n & 15, t, t_lo, sp, cp, &num_lo);
	den = __libc_fast_sin_turn((n & 15) + 16, t, t_lo, sp, cp, &den_lo);
	if ((n & 16) != 0) {
		double swap = num;
		double swap_lo = num_lo;

		num = -den;
		num_lo = -den_lo;
		den = swap;
		den_lo = swap_lo;
	}

	/* The kernel's low parts reach 2^-10 of the high ones: normalized, so the quotient's remainder needs only first order */
	product = __libc_dd_fast_sum(num, num_lo);
	num = product.hi;
	num_lo = product.lo;
	product = __libc_dd_fast_sum(den, den_lo);
	den = product.hi;
	den_lo = product.lo;

	q = num / den;
	product = __libc_dd_product(q, den);
	r = (((num - product.hi) - product.lo) + (num_lo - q * den_lo)) / den;

	/* tan is odd */
	return __libc_fast_signed_sum(q, r, signbit(x) != 0);
}
