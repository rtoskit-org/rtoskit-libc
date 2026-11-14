/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2.c
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
 * The fast log2 of the soft-double cores (PLAN D162, D192): log's method, the series' result times log2(e) (in Q63,
 * normalized), and on the table path k exactly plus ln z log2(e) as ln z + ln z (log2(e) - 1), in 128 bits (the sum
 * can reach 2^-1). A power of 2 is k exactly (ln z's error is far below half an ulp of k). C23 7.12.6.13, F.10.3.9.
 */
double log2(double x)
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
		n = __libc_dp_fixed_scale(n, 0xb8aa3b295c17f0bcull, &e);
		return __libc_dp_fixed_double(negative ? 0x8000000000000000ull : 0, e, n);
	}

	t = __libc_dp_fixed_log_core(z);
	u = (int64_t)__libc_trig_high64(t < 0 ? -(uint64_t)t : (uint64_t)t, 0x71547652b82fe177ull);
	hi = (uint64_t)(int64_t)k;
	lo = 0;
	__libc_dp_fixed_add128(&hi, &lo, t);
	__libc_dp_fixed_add128(&hi, &lo, t < 0 ? -u : u);

	return __libc_dp_fixed_from128(hi, lo);
}
