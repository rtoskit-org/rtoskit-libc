/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_tgamma_negative.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * A negative x, not an integer, from 2^-54 in magnitude: a = -x, n = floor(a), the sign (-1)^(n + 1).
 * - Above -8, the recurrence Gamma(x) = Gamma(z)/(x (x + 1) ... (x + n + 1)), z = x + n + 2 in (1, 2): the piece's t =
 *   x - (m - n - 2) (exact from |x| >= 1; below, its rounding only moves t), the product of the shifts as a pair (x + 1
 *   a pair below 1 in magnitude), one quotient.
 * - From -8, the reflection Gamma(-a) = -pi/(a sin(pi a) Gamma(a)) as one exponential, of log pi - log(a |sin pi g|) -
 *   E(a) with E Stirling's exponent, so that the underflow (from -184) rounds once; sin(pi g) by the fast sine's table and
 *   kernel, g the distance from a to the nearest integer.
 */
double __libc_fast_tgamma_negative(double x)
{
	double a = -x;
	double n = __libc_fast_tgamma_floor(a);
	double fraction = a - n;
	double sign;

	if (fraction == 0)
		return __libc_math_invalid();
	/* (-1)^(n + 1), n's parity by exact operations (no integer conversion, whose library call raised a flag on armv7e-m-dp) */
	sign = __libc_fast_tgamma_floor(n * 0.5) == n * 0.5 ? -1.0 : 1.0;
	/* |Gamma(x)| is below 2^-1075 from 184 */
	if (a >= 190.0)
		return __libc_math_underflow(sign);
	if (a < 8.0) {
		double m = n + 2.0;
		const double *c = __libc_fast_tgamma_piece(x + m);
		struct __libc_dd p = __libc_dd_sum(x, 0.0);
		double g_lo;
		double g = __libc_fast_tgamma_eval(c, x - (c[0] - m), &g_lo);

		for (double k = 1.0; k < m; k += 1.0) {
			struct __libc_dd v = __libc_dd_sum(x, k);
			struct __libc_dd q = __libc_fast_product(p.hi, v.hi);

			p = __libc_dd_fast_sum(q.hi, q.lo + p.lo * v.hi + p.hi * v.lo);
		}
		return __libc_fast_tgamma_divide(g, g_lo, p.hi, p.lo);
	} else {
		double g = fraction <= 0.5 ? fraction : 1.0 - fraction;
		uint32_t j = (uint32_t)(g * 32.0 + 0.5);
		double d = g - (double)j * 0x1p-5;
		struct __libc_dd t = __libc_fast_product(d, 0x1.921fb54442d18p+1);
		double z;
		double s_lo;
		double s;
		struct __libc_dd q;
		double l_lo;
		double l;
		double e_lo;
		double e;
		struct __libc_dd first;
		struct __libc_dd second;

		t.lo += d * 0x1.1a62633145c07p-53;
		z = t.hi * t.hi;
		s = __libc_fast_sin_turn(j, t.hi, t.lo, __libc_fast_sin_poly(z), __libc_fast_cos_poly(z), &s_lo);
		q = __libc_fast_product(a, s);
		q = __libc_dd_fast_sum(q.hi, q.lo + a * s_lo);
		l = __libc_fast_log_pair(q.hi, q.lo, 0, &l_lo);
		e = __libc_fast_tgamma_stirling(a, &e_lo);
		first = __libc_dd_sum(0x1.250d048e7a1bdp+0, -l);
		second = __libc_dd_sum(first.hi, -e);
		return __libc_fast_tgamma_exp(second.hi, second.lo + first.lo + (0x1.7abf2ad8d5088p-57 - l_lo - e_lo), sign);
	}
}
