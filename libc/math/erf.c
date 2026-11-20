/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erf.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/* C23 7.12.8.1, F.10.5.1: two terms below 2^-28, the series below 1/2, 1 - erfc above, +-1 (inexact) from 6 */
double erf(double x)
{
	const struct __libc_dd two_sqrtpi = { .hi = 0x1.20dd750429b6dp+0, .lo = 0x1.1ae3a914fed80p-56 };
	double a = fabs(x);
	struct __libc_dd value;
	struct __libc_dd tail;
	int exponent;
	int mode;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return copysign(1.0, x);
	if (a < 0x1p-28) {
		/*
		 * x 2/sqrt(pi) + x^3 p0 = x (2/sqrt(pi) + x^2 p0), p0 = -2/(3 sqrt(pi)): x^2 p0 is up to 2^-57.6 of the constant,
		 * which dropped cost 0.53 ulp (Q57), so it joins the constant's low part, and the product is exact as a pair
		 * rounded once through the 2^200 scaling (subnormal results round once). Below 2^-300 it is far below the
		 * last place and is left out, so that x x raises no spurious underflow.
		 */
		const double p0 = -0x1.812746b0379e7p-2;
		double square = a < 0x1p-300 ? 0.0 : x * x;

		return __libc_dd_scale_product(x, (struct __libc_dd){ .hi = two_sqrtpi.hi, .lo = two_sqrtpi.lo + square * p0 });
	}
	if (a >= 6.0) {
		volatile double tiny = 0x1p-80;

		return x > 0 ? 1.0 - tiny : tiny - 1.0;
	}

	mode = __libc_math_nearest();
	if (a < 0.5) {
		value = __libc_erf_series(a);
	} else if (a < 4.0) {
		value = __libc_dd_add((struct __libc_dd){ .hi = 1.0, .lo = 0.0 }, __libc_dd_negate(__libc_erfc_table(a)));
	} else {
		tail = __libc_erfc_tail(a, &exponent);
		value = __libc_dd_add((struct __libc_dd){ .hi = 1.0, .lo = 0.0 }, (struct __libc_dd){ .hi = -scalbn(tail.hi, exponent), .lo = -scalbn(tail.lo, exponent) });
	}
	if (signbit(x))
		value = __libc_dd_negate(value);
	__libc_math_restore(mode);

	return value.hi + value.lo;
}
