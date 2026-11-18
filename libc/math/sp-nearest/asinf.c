/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinf.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"

/* C23 7.12.4.2: atan2(x, sqrt((1 - x)(1 + x))) with the radicand exact and the root corrected (PLAN D61) */
float asinf(float x)
{
	struct __libc_ff value;

	if (isnan(x))
		return x + x;
	if (isgreater(fabsf(x), 1.0f))
		return __libc_math_invalidf();
	/* The x^3/6 term is below half an ulp of x */
	if (fabsf(x) < 0x1p-12f)
		return x;

	value = __libc_atan2f_core((struct __libc_ff){ .hi = x, .lo = 0.0f }, __libc_ff_sqrt(__libc_ff_multiply(__libc_ff_sum(1.0f, -x), __libc_ff_sum(1.0f, x))));

	return value.hi + value.lo;
}

/* The float complex layer's accurate asinf (D236): this one, within 0.52 ulp */
float __libc_asinf_accurate(float x) __attribute__((__alias__("asinf")));
