/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincosf.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

#include "internal/math.h"

/*
 * GNU sincosf (P8.02, docs/research/sincos.md): sinf's and cosf's entry tests
 * on one argument, one reduction, then the core twice (cos x = sin(x + pi/2),
 * k + 16), so each result is bit-identical to sinf's and cosf's
 */
void sincosf(float x, float *sinp, float *cosp)
{
	struct __libc_ff s;
	struct __libc_ff value;
	int k;

	if (isnan(x)) {
		*sinp = x + x;
		*cosp = x + x;
		return;
	}
	if (isinf(x)) {
		*sinp = __libc_math_invalidf();
		*cosp = __libc_math_invalidf();
		return;
	}

	if (fabsf(x) < 0x1p-6f) {
		k = 0;
		s = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	} else {
		k = __libc_rem_pio2f(x, &s);
	}

	/* sinf: the x^3 term is below half an ulp of x, so the result rounds to x (keeping -0) */
	if (fabsf(x) < 0x1p-12f) {
		*sinp = x;
	} else {
		value = __libc_sinf_core(k, s);
		*sinp = value.hi + value.lo;
	}
	value = __libc_sinf_core(k + 16, s);
	*cosp = value.hi + value.lo;
}
