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
 * The ends (P11.08d1, D246), out of line so that the common path keeps no stack frame: up to about 2^-7.5 in magnitude x +
 * x^2 P(x) (log's polynomial, relative however small x is; below 2^-60 x nudged toward -inf, underflow for a
 * subnormal; a quiet NaN takes it and raises nothing), log1p(-1) = -inf with divide-by-zero, below -1 a domain error,
 * from 2^1022 log x, and +inf. C23 7.12.6.14, F.10.3.10.
 */
[[gnu::noinline]] static double log1p_ends(double x)
{
	/* Below the common path's bound, 0x1.6a09fp-8 (its high word), just above 2^-7.5 */
	if (!isgreaterequal(__builtin_fabs(x), 0x1.6a09fp-8))
		return isless(__builtin_fabs(x), 0x1p-60) ? __libc_math_tiny(x, -1.0) : x + x * x * __libc_fast_log_poly(x, x * x);
	if (x == -1.0)
		return __libc_math_divbyzero(-1.0);
	/* From 2^1022 log1p x is log x within 2^-1022 relatively (and 1 + x could round up to infinity) */
	if (x > 0)
		return isinf(x) ? x : log(x);

	return __libc_math_invalid();
}

/*
 * The fast log1p of the double-FPU cores (PLAN D162, D190, D246): u = 1 + x rounded with its error e exact (a fast
 * two-sum, the larger magnitude first, exact at any size), log(u) as log's, and e/u added to the tail. One integer
 * test on the high word sends 2^-7.5 < |x| with -1 < x < 2^1022 down the common path (below 2^1022 e/u cannot
 * underflow, regression_log1p_large), the rest to log1p_ends.
 */
double log1p(double x)
{
	const struct __libc_fast_log_entry *entry;
	uint64_t bits = __libc_double_bits(x);
	uint32_t high = (uint32_t)(bits >> 32) & 0x7fffffff;
	uint32_t top = (bits >> 63) != 0 ? 0x3ff00000u : 0x7fd00000u;
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

	if (high - 0x3f76a09fu >= top - 0x3f76a09fu)
		return log1p_ends(x);

	u = 1.0 + x;
	e = high >= 0x3ff00000u ? (x - u) + 1.0 : (1.0 - u) + x;
	entry = __libc_fast_log_reduce(__libc_fast_log_table, __libc_double_bits(u), &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	kd = (double)k;
	w = kd * 0x1.62e42fefa3800p-1 + entry->logc;
	hi = w + r;
	lo = (w - hi) + r + (kd * 0x1.ef35793c76730p-45 + entry->tail + e / u);
	r2 = r * r;

	return lo + r2 * __libc_fast_log_poly(r, r2) + hi;
}
