/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* The ends: +-0, +-inf and NaN return themselves, below 2^-27 x nudged toward zero (underflow for a subnormal) */
[[gnu::noinline]] static double asinh_ends(double x)
{
	if (isnan(x) || isinf(x) || x == 0)
		return x + x;

	return __libc_math_tiny(x, -x);
}

/*
 * The fast asinh of the soft-double cores (PLAN D162, D203), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, |x| = M 2^e from 2^-27:
 * - below 1, log1p(w) with w = |x| + x^2/(1 + sqrt(1 + x^2)): 1 + x^2 in Q62, its root (__libc_dp_fixed_root), the
 *   quotient by the 64-bit reciprocal, each to about 2^-62 relatively, and log1p of the sum (relative below 2^-5);
 * - from 1, log(|x| + sqrt(x^2 + 1)) by the table path (the result is at least 0.88);
 * - from 2^28, log 2|x| (the term 1/(4 x^2) is below 2^-58).
 * C23 7.12.5.2, F.10.2.2.
 */
double asinh(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t a = ((magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
	uint64_t p;
	uint64_t h;
	uint64_t r;
	uint64_t q;
	int32_t f;
	int32_t k;

	/* 2^-27 <= |x|, finite */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x7ff))
		return asinh_ends(x);
	if (e >= 28)
		return __libc_double_from(__libc_double_bits(__libc_dp_fixed_log_scaled(a, e + 1)) | sign);

	/* M^2 in Q62 */
	p = __libc_trig_high64(a, a);
	if (e >= 0) {
		/* x^2 + 1 = (M^2/2 + 2^(-2e - 1)) 2^(2e + 1), normalized as h 2^(2e + 3 - k - 64); its root r 2^(f - 64) */
		h = (p >> 1) + (1ull << (61 - 2 * e));
		k = __builtin_clzll(h);
		r = __libc_dp_fixed_sqrt_scaled(h << k, 2 * e + 3 - k, &f);
		/* |x| + r at 2^(f - 63), r (above |x|) 2^(f - 1 - 63) */
		h = (r >> 1) + (a >> (f - e));
		k = __builtin_clzll(h);
		return __libc_double_from(__libc_double_bits(__libc_dp_fixed_log_scaled(h << k, f - k)) | sign);
	}

	/* 1 + x^2 in Q62, in [2^62, 2^63); r = sqrt(1 + x^2) in Q63 */
	r = __libc_dp_fixed_root(0x4000000000000000ull + (-2 * e < 64 ? p >> (-2 * e) : 0));
	/* x^2/(1 + r) = (M^2/(1 + r)) 2^2e: q = p 2^k / ((1 + r) 2^62) in Q63, normalized */
	k = __builtin_clzll(p);
	q = __libc_trig_high64(p << k, __libc_dp_fixed_reciprocal(0x4000000000000000ull + (r >> 1)));
	f = 2 * e - k - __builtin_clzll(q);
	q <<= __builtin_clzll(q);
	/* w = |x| + q 2^(f - 63) at 2^(e + 1 - 63), q at most |x|/2 */
	h = (a >> 1) + (e + 1 - f < 64 ? q >> (e + 1 - f) : 0);
	k = __builtin_clzll(h);

	return __libc_double_from(__libc_double_bits(__libc_dp_fixed_log1p_scaled(h << k, e + 1 - k)) | sign);
}
