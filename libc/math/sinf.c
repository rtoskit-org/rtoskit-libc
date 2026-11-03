/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sinf.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"

/* C23 7.12.4.6: evaluated in double, then rounded once more; tiny arguments round by hand (docs/research/libm-trig.md) */
float sinf(float x)
{
	if (isless(fabsf(x), 0x1p-30f))
		return __libc_math_tinyf(x, -x);

	return __libc_float_round(sin(x), x, __libc_sinf_cases, __LIBC_SINF_CASES);
}
