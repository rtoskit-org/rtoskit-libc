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
#include <stdbool.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * The fast log1p of the soft-double cores (PLAN D162, D192), in 64-bit fixed point: below 2^-5 the relative series on
 * x itself; otherwise 1 + x as 2^k z in integers (exact below 2^11 and for x < 0, else to 64 bits, 2^-64 relatively)
 * and log's table path. Below 2^-60 x nudged toward -inf (underflow for a subnormal); log1p(-1) is -inf with
 * divide-by-zero, below -1 a domain error (C23 7.12.6.14, F.10.3.10).
 */
double log1p(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t magnitude = bits & 0x7fffffffffffffffull;
	uint64_t significand = ((bits << 12) >> 1) | 0x8000000000000000ull;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t hi;
	uint64_t lo;
	uint64_t z;
	int32_t k;

	if (magnitude >= 0x7ff0000000000000ull) {
		if (magnitude > 0x7ff0000000000000ull)
			return x + x;
		return bits == magnitude ? x : __libc_math_invalid();
	}
	if (magnitude < 0x3fa0000000000000ull) {
		uint64_t n;

		if (magnitude < 0x3c30000000000000ull)
			return __libc_math_tiny(x, -1.0);
		n = __libc_dp_fixed_log_near(significand, &e, bits != magnitude);
		return __libc_dp_fixed_double(bits & 0x8000000000000000ull, e, n);
	}

	if (bits != magnitude) {
		uint64_t d;
		int32_t shift;

		if (magnitude >= 0x3ff0000000000000ull)
			return magnitude == 0x3ff0000000000000ull ? __libc_math_divbyzero(-1.0) : __libc_math_invalid();
		/* 1 - |x| exactly: |x| in [2^-5, 1), its significand shifted by at most 5 of its 11 zero bits */
		d = 0x8000000000000000ull - (significand >> -e);
		shift = __builtin_clzll(d);
		z = d << shift;
		k = -shift;
	} else if (e < 0) {
		z = 0x8000000000000000ull + (significand >> -e);
		k = 0;
	} else if (e < 64) {
		/* 1.m 2^e + 1 in Q63 at 2^e, a carry into a bit 64 taken back (its last bit dropped) */
		z = significand + (0x8000000000000000ull >> e);
		k = e;
		if (z < significand) {
			z = (z >> 1) | 0x8000000000000000ull;
			k++;
		}
	} else {
		z = significand;
		k = e;
	}
	if (z >= 0xb400000000000000ull) {
		z >>= 1;
		k++;
	}

	__libc_dp_fixed_k_signed(k, 0xb17217f7d1cf79acull, &hi, &lo);
	__libc_dp_fixed_add128(&hi, &lo, __libc_dp_fixed_log_core(z));

	return __libc_dp_fixed_from128(hi, lo);
}
