/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * The fast exp2 of the soft-double cores (PLAN D162, D188), in 64-bit fixed point with one rounding: t = x itself as n + f
 * 2^-64 by shifting x's significand (below 2^-12 its last bits are dropped, below 2^-63 of the result), negated for x < 0,
 * and 2^t by the kernel. The common path is 2^-54 <= |x| < 512; the slow path is the strict file's. C23 7.12.6.4,
 * F.10.3.2.
 */
double exp2(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t m = (bits & 0x000fffffffffffffull) | 0x0010000000000000ull;
	int e = (int)((bits >> 52) & 0x7ff) - 1023;
	uint64_t f;
	int32_t n = 0;

	if (__libc_fast_exp_outside(x, 0x3c9, 0x408))
		return __libc_fast_exp2_special(x);

	/* |x| 2^64 = m 2^(e + 12): the integer part from m's top bits, the fraction from the rest */
	if (e >= 0) {
		n = (int32_t)(m >> (52 - e));
		f = m << (12 + e);
	} else {
		f = e >= -12 ? m << (12 + e) : m >> (-12 - e);
	}
	if ((bits >> 63) != 0)
		f = __libc_dp_fixed_negate(&n, f);

	return __libc_dp_fixed_exp2(n, f);
}
