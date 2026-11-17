/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * lgamma.c
 *
 * Created on: Nov 16, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast lgamma of the double-FPU cores (PLAN D162, D219, D225), within 1 ulp, and near the zeros far within 2^-60
 * absolutely: one test of |x|'s bits sends 0, below 2^-60, from 2^60, the infinities and NaN to __libc_fast_lgamma_ends,
 * a negative x to __libc_fast_lgamma_negative, the rest to __libc_fast_lgamma_positive (their files; the soft-double
 * cores' fixed-point file shares the first two). C23 7.12.8.3.
 */
double lgamma(double x)
{
	uint64_t magnitude = __libc_double_bits(x) & 0x7fffffffffffffffull;
	double lo;
	double hi;

	if (magnitude - 0x3c30000000000000ull >= 0x43b0000000000000ull - 0x3c30000000000000ull)
		return __libc_fast_lgamma_ends(x);
	if (x < 0)
		return __libc_fast_lgamma_negative(x);
	hi = __libc_fast_lgamma_positive(x, &lo);

	return hi + lo;
}
