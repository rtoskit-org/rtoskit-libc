/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log.c
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
 * The fast log of the soft-double cores (PLAN D162, D192), in 64-bit fixed point with one rounding and no soft-double
 * operation: x = 2^k z with z in [45/64, 90/64); within 2^-5 of 1 the relative series (__libc_dp_fixed_log_near),
 * otherwise k ln 2 (ln 2 in Q64, the product in 128 bits) plus ln z from the table (__libc_dp_fixed_log_core), rounded
 * from 128 bits. log(1) = +0; x <= 0, +inf and NaN out of line (__libc_fast_log_special). C23 7.12.6.11, F.10.3.7.
 */
double log(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t hi;
	uint64_t lo;
	uint64_t n;
	uint64_t z;
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
		return __libc_dp_fixed_double(negative ? 0x8000000000000000ull : 0, e, n);
	}

	__libc_dp_fixed_k_signed(k, 0xb17217f7d1cf79acull, &hi, &lo);
	__libc_dp_fixed_add128(&hi, &lo, __libc_dp_fixed_log_core(z));

	return __libc_dp_fixed_from128(hi, lo);
}
