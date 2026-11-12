/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_exp2.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/*
 * 2^(n + f) = 2^n 2^(j/64) 2^r with j the top 6 bits of f: y = T (1 + P) in Q63 (__libc_dp_fixed_exp2_fraction), rounded
 * to 53 bits by its 54th, a carry going into the exponent (which the implicit bit already raises by one)
 */
double __libc_dp_fixed_exp2(int32_t n, uint64_t f)
{
	uint64_t y = __libc_dp_fixed_exp2_fraction(f);

	return __libc_double_from(((uint64_t)(int64_t)(n + 1022) << 52) + (y >> 11) + ((y >> 10) & 1));
}
