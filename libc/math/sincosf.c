/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

#include "internal/math.h"

/*
 * GNU sincosf on the double path (P8.02): the double sincos (one reduction),
 * then each result rounded as sinf and cosf round theirs, with their hard
 * cases, so both are bit-identical to sinf and cosf
 */
void sincosf(float x, float *sinp, float *cosp)
{
	double sine;
	double cosine;

	/* sinf's own tiny-argument path; the cosine still comes from cos */
	if (isless(fabsf(x), 0x1p-30f)) {
		*sinp = __libc_math_tinyf(x, -x);
		*cosp = __libc_float_round(cos(x), x, __libc_cosf_cases, __LIBC_COSF_CASES);
		return;
	}

	sincos(x, &sine, &cosine);
	*sinp = __libc_float_round(sine, x, __libc_sinf_cases, __LIBC_SINF_CASES);
	*cosp = __libc_float_round(cosine, x, __libc_cosf_cases, __LIBC_COSF_CASES);
}
