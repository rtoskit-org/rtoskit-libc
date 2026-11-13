/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_log_special.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/* x <= 0, +inf or NaN: log(+-0) is -inf with divide-by-zero, a negative x a domain error (C23 7.12.6.11, F.10.3.7) */
double __libc_fast_log_special(double x)
{
	if (isnan(x))
		return x + x;
	if (x == 0)
		return __libc_math_divbyzero(-1.0);
	if (x < 0)
		return __libc_math_invalid();

	return x;
}
