/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_tgamma_large.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/* tgamma(x) for 8 <= x <= 171.62437695630272: e^E with Stirling's exponent as a pair, rounded once (PLAN D230) */
double __libc_fast_tgamma_large(double x)
{
	double lo;
	double hi = __libc_fast_tgamma_stirling(x, &lo);

	return __libc_fast_tgamma_exp(hi, lo, 1.0);
}
