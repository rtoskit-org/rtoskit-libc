/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asin.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast asin of the soft-double cores (PLAN D162, D200; docs/research/libm-fast-double.md, section 18): the fixed-point
 * kernel, one rounding. C23 7.12.4.2, F.10.1.2: |x| > 1 is a domain error; +-0 returns itself (the tiny path, below 2^-27);
 * asin(+-1) is +-pi/2.
 */
double asin(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;

	if (magnitude >= 0x3ff0000000000000ull) {
		if (magnitude == 0x3ff0000000000000ull)
			return __libc_double_from(sign | 0x3ff921fb54442d18ull);
		return magnitude > 0x7ff0000000000000ull ? x + x : __libc_math_invalid();
	}
	if (magnitude < 0x3e40000000000000ull)
		return __libc_math_tiny(x, x);

	return __libc_dp_fixed_asin(magnitude, false, sign != 0);
}
