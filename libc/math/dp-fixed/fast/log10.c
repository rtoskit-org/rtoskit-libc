/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * The fast log10 of the soft-double cores (PLAN D162, D192): log's method, the series' result times log10(e) (4 log10(e)
 * in Q63, normalized), and on the table path k log10(2) (in Q64, the product in 128 bits) plus ln z log10(e). Powers of
 * 10 come out exact. C23 7.12.6.12, F.10.3.8.
 */
double log10(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t hi;
	uint64_t lo;
	uint64_t n;
	uint64_t z;
	int64_t t;
	int64_t u;
	int32_t k;

	if (bits - 1 >= 0x7fefffffffffffffull)
		return __libc_fast_log_special(x);

	z = __libc_dp_fixed_log_reduce(bits, &k);
	if (k == 0 && z - 0x7c00000000000001ull < 0x07ffffffffffffffull) {
		/* Strictly within 2^-5 of 1 (the series needs |s| < 2^-5): s = z - 1 exactly, normalized, and the relative series */
		bool negative = z < 0x8000000000000000ull;
		uint64_t s = negative ? 0x8000000000000000ull - z : z - 0x8000000000000000ull;
		int32_t shift;
		int32_t e;

		if (s == 0)
			return 0.0;
		shift = __builtin_clzll(s);
		e = -shift;
		n = __libc_dp_fixed_log_near(s << shift, &e, negative);
		n = __libc_dp_fixed_scale(n, 0xde5bd8a937287195ull, &e);
		return __libc_dp_fixed_double(negative ? 0x8000000000000000ull : 0, e - 2, n);
	}

	t = __libc_dp_fixed_log_core(z);
	u = (int64_t)__libc_trig_high64(t < 0 ? -(uint64_t)t : (uint64_t)t, 0x6f2dec549b9438cbull);
	__libc_dp_fixed_k_signed(k, 0x4d104d427de7fbccull, &hi, &lo);
	__libc_dp_fixed_add128(&hi, &lo, t < 0 ? -u : u);

	return __libc_dp_fixed_from128(hi, lo);
}
