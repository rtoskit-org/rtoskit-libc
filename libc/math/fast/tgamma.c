/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgamma.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast tgamma of the double-FPU cores (PLAN D162, D219, D230), within 1 ulp (docs/research/libm-fast-double.md,
 * section 34):
 * - the integers to 23 exactly ((n - 1)!);
 * - below 1, Gamma(1 + x)/x, Gamma(1 + x) by the piece of 1 + x at t = x - (m - 1), one quotient;
 * - below 8, the pieces;
 * - to 171.6, e^E with E Stirling's exponent as a pair, rounded once;
 * - negative x by the recurrence above -8 and the reflection below; those paths, the ends and Stirling's in their own
 *   files, which the soft-double cores' fixed-point file shares (__libc_fast_tgamma_*.c).
 * C23 7.12.8.4.
 */
double tgamma(double x)
{
	uint64_t magnitude = __libc_double_bits(x) & 0x7fffffffffffffffull;
	const double *c;
	double lo;
	double hi;

	/* 2^-54 <= |x| < 2^52, and x below the overflow */
	if (magnitude - 0x3c90000000000000ull >= 0x4330000000000000ull - 0x3c90000000000000ull || x > 171.62437695630272)
		return __libc_fast_tgamma_ends(x);
	if (x < 0)
		return __libc_fast_tgamma_negative(x);
	if (x <= 23.0 && (double)(int32_t)x == x)
		return __libc_fast_tgamma_factorials[(int32_t)x - 1];
	if (x < 1.0) {
		c = __libc_fast_tgamma_piece(1.0 + x);
		hi = __libc_fast_tgamma_eval(c, x - (c[0] - 1.0), &lo);
		return __libc_fast_tgamma_divide(hi, lo, x, 0.0);
	}
	if (x < 8.0) {
		c = __libc_fast_tgamma_piece(x);
		hi = __libc_fast_tgamma_eval(c, x - c[0], &lo);
		return hi + lo;
	}
	return __libc_fast_tgamma_large(x);
}
