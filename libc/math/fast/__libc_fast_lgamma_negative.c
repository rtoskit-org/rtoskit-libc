/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_lgamma_negative.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* floor(a) for a positive finite a, by masking its fraction bits: exact in every rounding direction, which the strict lgammaf (calling lgamma) needs */
static inline double lgamma_floor(double a)
{
	uint64_t bits = __libc_double_bits(a);
	int32_t e = (int32_t)(bits >> 52) - 1023;

	if (e < 0)
		return 0.0;
	if (e >= 52)
		return a;

	return __libc_double_from(bits & ~((1ull << (52 - e)) - 1));
}

/*
 * A negative x in (-16, -1), not an integer, by the recurrence gamma(x) = gamma(z)/(x (x + 1) ... (x + n + 1)) with
 * x in (-n - 1, -n) and z = x + n + 2 in (1, 2), exact (|x| >= 1): lgamma(x) = lgamma(z) - log P, P = a (a - 1) ...
 * (a - n) (n + 1 - a) with a = -x, every factor exact and P a normalized pair. Near every zero both terms are small
 * (P near 1, z near 1 or 2), so neither cancels the other: relative however close to a zero (the reflection's terms are
 * near log n! there, and their difference lost up to 1.5 ulp from -8, D225). At most 16 pair products.
 */
[[gnu::noinline]] static double lgamma_product(double a)
{
	double n = lgamma_floor(a);
	struct __libc_dd p = { .hi = a, .lo = 0.0 };
	struct __libc_dd product;
	struct __libc_dd sum;
	double l_lo;
	double l;
	double g_lo;
	double g;
	double v;

	for (double k = 1.0; k <= n + 1.0; k += 1.0) {
		v = k <= n ? a - k : k - a;
		product = __libc_fast_product(p.hi, v);
		p = __libc_dd_fast_sum(product.hi, product.lo + p.lo * v);
	}
	l = __libc_fast_log_pair(p.hi, p.lo, 0, &l_lo);
	g = __libc_fast_lgamma_positive(n + 2.0 - a, &g_lo);
	sum = __libc_dd_sum(g, -l);

	return sum.hi + (sum.lo + (g_lo - l_lo));
}

/*
 * A negative x with |x| from 2^-60 (in (-16, -1) by lgamma_product), else by the reflection gamma(-a) gamma(a) = -pi/(a sin(pi a)), a = -x: lgamma(x) =
 * log pi - log(a |sin(pi f)|) - lgamma(a), |f| = the distance from a to the nearest integer, exact (lgamma_floor; 0 only at the poles, divide-by-zero), |sin(pi f)| = sin(N pi/32 + t) by the fast sin's table and kernel (N =
 * round(32 |f|), t = pi (|f| - N/32) as a pair), a sin as a pair into the log's.
 */
double __libc_fast_lgamma_negative(double x)
{
	double a = -x;
	double fraction = a - lgamma_floor(a);
	/* |f| = the distance to the nearest integer (1 - fraction exact from 1/2) */
	double g = fraction <= 0.5 ? fraction : 1.0 - fraction;
	double kd;
	uint32_t k;
	double d;
	struct __libc_dd t;
	struct __libc_dd q;
	struct __libc_dd first;
	struct __libc_dd second;
	double s_lo;
	double s;
	double l_lo;
	double l;
	double r_lo;
	double r;
	double z;

	if (g == 0)
		return __libc_math_divbyzero(1.0);
	if (a > 1.0 && a < 16.0)
		return lgamma_product(a);
	/* N by truncation, not a rounding shift (any rounding direction): off by one only within 2^-53 of a half, which the kernel's |t| bound absorbs */
	k = (uint32_t)(g * 32.0 + 0.5);
	kd = (double)k;
	d = g - kd * 0x1p-5;
	t = __libc_fast_product(d, 0x1.921fb54442d18p+1);
	t.lo += d * 0x1.1a62633145c07p-53;
	z = t.hi * t.hi;
	s = __libc_fast_sin_turn(k, t.hi, t.lo, __libc_fast_sin_poly(z), __libc_fast_cos_poly(z), &s_lo);
	q = __libc_fast_product(a, s);
	q = __libc_dd_fast_sum(q.hi, q.lo + a * s_lo);
	l = __libc_fast_log_pair(q.hi, q.lo, 0, &l_lo);
	r = __libc_fast_lgamma_positive(a, &r_lo);
	first = __libc_dd_sum(0x1.250d048e7a1bdp+0, -l);
	second = __libc_dd_sum(first.hi, -r);

	return second.hi + (second.lo + first.lo + (0x1.7abf2ad8d5088p-57 - l_lo - r_lo));
}
