/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_cos_core.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * cos(r) = 1 - 2 sin^2(r/2) for |r| <= 0.8: halving is exact, sin(r/2) is
 * then good to about 2^-66, and the square and the difference are
 * double-double. Summing the Taylor series of cos in double instead left
 * r^4/24, up to 2^-6, to double rounding: 0.06 ulp near pi/4.
 */
struct __libc_dd __libc_cos_core(struct __libc_dd r)
{
	struct __libc_dd half = __libc_sin_core((struct __libc_dd){ .hi = 0.5 * r.hi, .lo = 0.5 * r.lo });
	struct __libc_dd square = __libc_dd_multiply(half, half);
	struct __libc_dd sum = __libc_dd_fast_sum(1.0, -2.0 * square.hi);

	return __libc_dd_fast_sum(sum.hi, sum.lo - 2.0 * square.lo);
}
