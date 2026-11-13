/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast atan of the soft-double cores (PLAN D162, D199; docs/research/libm-fast-double.md, section 17): K(|x|, 1) for
 * |x| <= 1 and pi/2 - K(1, |x|) above, from the fixed-point kernel, one rounding. C23 7.12.4.3, F.10.1.3: atan(+-inf) is
 * +-pi/2, as is every |x| from 2^60 once rounded; +-0 returns itself (the tiny path, below 2^-27).
 */
double atan(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = (magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull;

	/* NaN, then +-inf and |x| from 2^60: pi/2 rounded, with x's sign */
	if (magnitude > 0x7ff0000000000000ull)
		return x + x;
	if (e >= 60)
		return __libc_double_from(sign | 0x3ff921fb54442d18ull);
	if (e < -27)
		return __libc_math_tiny(x, -x);

	if (magnitude <= 0x3ff0000000000000ull)
		return __libc_dp_fixed_atan(m, e, 0x0010000000000000ull, 0, 0, sign != 0);

	return __libc_dp_fixed_atan(0x0010000000000000ull, 0, m, e, 1, sign != 0);
}
