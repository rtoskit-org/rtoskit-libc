/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/*
 * The fast exp10 of the soft-double cores (PLAN D162, D188), in 64-bit fixed point with one rounding: t = |x| log2(10) from
 * x's significand times log2(10) to 128 bits, negated for x < 0, and 2^t by the kernel. The common path is 2^-54 <= |x| <
 * 256; the slow path is the strict file's. C23 7.12.6.2, F.10.3.3.
 */
double exp10(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t f;
	int32_t n;

	if (__libc_fast_exp_outside(x, 0x3c9, 0x407))
		return __libc_fast_exp10_special(x);

	f = __libc_dp_fixed_scaled((bits & 0x000fffffffffffffull) | 0x0010000000000000ull, (int)((bits >> 52) & 0x7ff) - 1023, 0xd49a784bcd1b8afeull, 0x492bf6ff4dafdb4cull, 62, &n);
	if ((bits >> 63) != 0)
		f = __libc_dp_fixed_negate(&n, f);

	return __libc_dp_fixed_exp2(n, f);
}
