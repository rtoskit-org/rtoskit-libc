/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log1p.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast log1p of the double-FPU cores (PLAN D162, D190): for |x| <= 2^-7.5, x + x^2 P(x) (log's polynomial, relative
 * however small x is); otherwise u = 1 + x rounded with its error e exact (a fast two-sum, the larger magnitude first;
 * above 2^53 u = x and e = 1, and from 2^60 e = 0), log(u) as log's, and e/u added to the tail. Below 2^-60 x nudged toward -inf (underflow for a subnormal); log1p(-1) is
 * -inf with divide-by-zero, below -1 a domain error (C23 7.12.6.14, F.10.3.10).
 */
double log1p(double x)
{
	const struct __libc_fast_log_entry *entry;
	double kd;
	double r2;
	double hi;
	double lo;
	double e;
	double u;
	double w;
	double z;
	double r;
	int32_t k;

	/* Quiet comparisons, so that a quiet NaN raises nothing; it takes the series, x + x^2 P(x) */
	if (!isgreater(__builtin_fabs(x), 0x1.6a09e667f3bcdp-8))
		return isless(__builtin_fabs(x), 0x1p-60) ? __libc_math_tiny(x, -1.0) : x + x * x * __libc_fast_log_poly(x, x * x);
	if (!(x > -1.0) || isinf(x)) {
		if (x == -1.0)
			return __libc_math_divbyzero(-1.0);
		return x > 0 ? x : __libc_math_invalid();
	}

	if (x > 0x1p53) {
		/* 1 + x exactly as x + 1, without 1 + x overflowing upward; from 2^60 1/x is below 2^-66 of the result */
		u = x;
		e = x > 0x1p60 ? 0.0 : 1.0;
	} else {
		u = 1.0 + x;
		e = __builtin_fabs(x) > 1.0 ? (x - u) + 1.0 : (1.0 - u) + x;
	}
	entry = __libc_fast_log_reduce(__libc_fast_log_table, __libc_double_bits(u), &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	kd = (double)k;
	w = kd * 0x1.62e42fefa3800p-1 + entry->logc;
	hi = w + r;
	lo = (w - hi) + r + (kd * 0x1.ef35793c76730p-45 + entry->tail + e / u);
	r2 = r * r;

	return lo + r2 * __libc_fast_log_poly(r, r2) + hi;
}
