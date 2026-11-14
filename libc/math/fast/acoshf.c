/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acoshf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast acoshf of the double-FPU cores (PLAN D162, D177): log(x + sqrt((x - 1)(x + 1))) in double, both factors exact,
 * the sum's rounding 2^-53 against a result of at least 2^-11.5; rounded once to float, below 0.51 ulp to nearest (the
 * trigonometric note, section 9). C23 F.10.2.1: below 1 invalid, acosh 1 = +0, +infinity itself.
 */
float acoshf(float x)
{
	double d = x;

	/* x < 1 (invalid), NaN, +infinity: as the double function */
	if (!isless(x, INFINITY) || !isgreaterequal(x, 1.0f))
		return (float)acosh(d);
	/* +0 in every rounding direction */
	if (x == 1.0f)
		return 0.0f;

	return (float)__libc_fast_log_positive(d + __builtin_sqrt((d - 1.0) * (d + 1.0)));
}
