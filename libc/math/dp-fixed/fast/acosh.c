/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acosh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* The ends: NaN returns itself, below 1 (and -inf) a domain error, 1 is +0, +inf itself */
[[gnu::noinline]] static double acosh_ends(double x)
{
	if (isnan(x))
		return x + x;
	if (x == 1.0)
		return 0.0;
	if (x < 1.0)
		return __libc_math_invalid();

	return x;
}

/*
 * The fast acosh of the soft-double cores (PLAN D162, D203), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, x = M 2^e:
 * - below 2, log1p(w) with w = t + sqrt(t (2 + t)), t = x - 1 exactly: the product to 2^-63, its root
 *   (__libc_dp_fixed_root), log1p of the sum, relative however close x is to 1 (w is at least 2^-25.5);
 * - from 2, log(x + sqrt(x^2 - 1)) by the table path (the result is at least 1.31);
 * - from 2^28, log 2x (the term -1/(4 x^2) is below 2^-58).
 * C23 7.12.5.1, F.10.2.1.
 */
double acosh(double x)
{
	uint64_t bits = __libc_double_bits(x);
	int32_t e = (int32_t)(bits >> 52) - 1023;
	uint64_t a = ((bits & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
	uint64_t t;
	uint64_t h;
	uint64_t r;
	int32_t f;
	int32_t k;
	int32_t j;

	/* 1 < x < +inf: the bits of a positive double grow with it */
	if (bits - 0x3ff0000000000001ull >= 0x7ff0000000000000ull - 0x3ff0000000000001ull)
		return acosh_ends(x);
	if (e >= 28)
		return __libc_dp_fixed_log_scaled(a, e + 1);

	if (e >= 1) {
		/* x^2 - 1 = (M^2/2 - 2^(-2e - 1)) 2^(2e + 1), normalized; its root r 2^(f - 64), below x */
		h = (__libc_trig_high64(a, a) >> 1) - (1ull << (61 - 2 * e));
		k = __builtin_clzll(h);
		r = __libc_dp_fixed_sqrt_scaled(h << k, 2 * e + 3 - k, &f);
		/* x + r at 2^(e + 1 - 63) */
		h = (a >> 1) + (r >> (e + 2 - f));
		k = __builtin_clzll(h);
		return __libc_dp_fixed_log_scaled(h << k, e + 1 - k);
	}

	/* t = x - 1 = t 2^(-j - 63) normalized; t (2 + t) = high(t, (2 + t) in Q62) 2^(-61 - j), normalized */
	t = a - 0x8000000000000000ull;
	j = __builtin_clzll(t);
	t <<= j;
	h = __libc_trig_high64(t, 0x8000000000000000ull + ((a - 0x8000000000000000ull) >> 1));
	k = __builtin_clzll(h);
	r = __libc_dp_fixed_sqrt_scaled(h << k, 3 - j - k, &f);
	/* w = t + r at 2^(f - 63), r (above t) 2^(f - 1 - 63) */
	h = (r >> 1) + (f + j < 64 ? t >> (f + j) : 0);
	k = __builtin_clzll(h);

	return __libc_dp_fixed_log1p_scaled(h << k, f - k);
}
