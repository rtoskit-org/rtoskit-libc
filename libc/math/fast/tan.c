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
 * The ends (P11.08d3, D248), out of line so that the common path keeps no stack frame: below 2^-27 x nudged away from
 * zero (underflow for a subnormal), NaN, the infinities (a domain error), and from 2^20 the slow path's reduction
 */
[[gnu::noinline]] static double tan_ends(double x)
{
	if (isless(__builtin_fabs(x), 0x1p-27))
		return __libc_math_tiny(x, x);
	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalid();

	return __libc_fast_trig_slow(x, 2);
}

/*
 * The fast tan of the double-FPU cores (PLAN D162, D196, D248; docs/research/libm-fast-double.md, section 42): a = j pi/32 +
 * t from the pi/32 reduction, j = (n + 8) mod 16 - 8 in [-8, 7], T = tan(|j| pi/32) from the table as a pair (nine
 * entries; for j < 0 a and the result negated) and u = tan t = t + t^3 P(t^2) (|t| <= pi/64, P of degree 3 fitted here, 2^-58.9 relative to t), then tan a = (T + u)/(1 -
 * T u), or -cot a = -(1 - T u)/(T + u) for an odd quadrant (bit 4 of n + 8), the period being pi: T + u by a fast
 * two-sum (|T| >= 0.098 >= |u| for j != 0), 1 - T u with T u exact and a fast two-sum (|T u| <= 0.05), and the quotient
 * with its remainder by a fused
 * multiply-add. One test on the high word sends 2^-27 <= |x| < 2^20 down this path. C23 7.12.4.7, F.10.1.7.
 */
double tan(double x)
{
	double a = __builtin_fabs(x);
	uint32_t high = (uint32_t)(__libc_double_bits(a) >> 32);
	const struct __libc_dd *e;
	struct __libc_dd num;
	struct __libc_dd den;
	struct __libc_dd p;
	struct __libc_dd swap;
	struct __libc_dd u;
	uint32_t n;
	int32_t j;
	bool negative;
	double u_lo;
	double t_lo;
	double t;
	double z;
	double q;
	double r;

	/* One test on the high word: 2^-27 <= |x| < 2^20 */
	if (high - 0x3e400000u >= 0x41300000u - 0x3e400000u)
		return tan_ends(x);

	n = __libc_fast_rem_pio32_nearest(a, &t, &t_lo);
	if (__libc_fast_trig_near_zero(n, t))
		return __libc_fast_trig_slow(x, 2);

	/* j = (n + 8) mod 16 - 8 in [-8, 7] and bit 4 of n + 8 the quadrant; tan and cot are odd, so for j < 0 a is negated
	 * (j and t) and so is the result */
	j = (int32_t)((n + 8) & 15) - 8;
	negative = signbit(x) != 0;
	if (j < 0) {
		j = -j;
		t = -t;
		t_lo = -t_lo;
		negative = !negative;
	}
	e = &__libc_fast_tan_table[j];
	z = t * t;
	u_lo = t_lo + t * (z * (0x1.55555555553bep-2 + z * (0x1.1111111423de7p-3 + z * (0x1.ba1b66bc85d39p-5 + z * 0x1.672e567178926p-6))));
	/* u normalized (its rest reaches 2^-20 of it, and the quotient takes the low parts to first order only) */
	u = __libc_dd_fast_sum(t, u_lo);
	/* T + u */
	num = __libc_dd_fast_sum(e->hi, u.hi);
	num.lo += e->lo + u.lo;
	/* 1 - T u */
	p = __libc_fast_fused_product(e->hi, u.hi);
	den = __libc_dd_fast_sum(1.0, -p.hi);
	den.lo -= p.lo + e->hi * u.lo + e->lo * u.hi;
	if (((n + 8) & 16) != 0) {
		swap = num;
		num.hi = -den.hi;
		num.lo = -den.lo;
		den = swap;
	}

	/* The quotient's remainder exact: one fused multiply-add, or the product as Dekker's pair without it */
	q = num.hi / den.hi;
	if (__libc_fpu_fma) {
		r = __builtin_fma(-q, den.hi, num.hi);
	} else {
		p = __libc_fast_product(q, den.hi);
		r = (num.hi - p.hi) - p.lo;
	}
	r = (r + (num.lo - q * den.lo)) / den.hi;

	/* tan is odd */
	return __libc_fast_signed_sum(q, r, negative);
}
