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
 * 2^r - 1 = sum of (r ln 2)^k/k! for r in [0, 1/64): with u = 64 r in Q64, the terms' coefficients (ln 2/64)^k/k! in Q64,
 * k = 1..7 (the eighth is below 0.09 units of 2^-64). All positive, so Horner's rule runs unsigned; the three highest fit
 * in 32 bits, so their products take two partial products, not four.
 */
#define A1 0x02c5c85fdf473de7ull
#define A2 0x0003d7f7bff058b2ull
#define A3 0x0000038d611ae094ull
#define A4 0x0000000276556df7ull
#define A5 0x00000000015d87feull
#define A6 0x000000000000a185ull
#define A7 0x0000000000000040ull

/*
 * 2^(n + f) = 2^n 2^(j/64) 2^r with j the top 6 bits of f: y = T (1 + P) in Q63, in [2^63, 2^64), its error a few units
 * (the table's half unit, the products' floors); rounded to 53 bits by its 54th, a carry going into the exponent (which the
 * implicit bit already raises by one)
 */
double __libc_dp_fixed_exp2(int32_t n, uint64_t f)
{
	uint64_t t = __libc_dp_fixed_exp2_table[f >> 58];
	uint64_t u = f << 6;
	uint64_t p;
	uint64_t y;

	p = A6 + __libc_trig_high64(u, A7);
	p = A5 + __libc_trig_high64(u, p);
	p = A4 + __libc_trig_high64(u, p);
	p = A3 + __libc_trig_high64(u, p);
	p = A2 + __libc_trig_high64(u, p);
	p = A1 + __libc_trig_high64(u, p);
	p = __libc_trig_high64(u, p);
	y = t + __libc_trig_high64(t, p);

	return __libc_double_from(((uint64_t)(int64_t)(n + 1022) << 52) + (y >> 11) + ((y >> 10) & 1));
}
