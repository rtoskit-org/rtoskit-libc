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
#include <stdint.h>

#include "internal/math.h"
#include "libm-fixed.h"

/* The fixed-point core with its rounding test, else the float-float core, as sinf and cosf (PLAN D86) */
static float one(int k, struct __libc_ff s)
{
	struct __libc_ff value;
	bool negative;
	int exponent;
	uint64_t v;
	uint32_t result;

	v = __libc_sinf_fixed(k, s.hi, s.lo, &exponent, &negative);
	if (v != 0 && __libc_fixed_round((uint32_t)(v >> 31), exponent, __LIBC_FIXED_SIN_ERROR, &result))
		return __libc_float_from(result | (negative ? 0x80000000u : 0));

	value = __libc_sinf_core(k, s);
	return value.hi + value.lo;
}

/*
 * GNU sincosf in the speed flavour (P8.02, docs/research/sincos.md): sinf's
 * and cosf's entry tests on one argument, one reduction, then each result as
 * sinf and cosf compute it, so both are bit-identical to them
 */
void sincosf(float x, float *sinp, float *cosp)
{
	struct __libc_ff s;
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
	*sinp = fabsf(x) < 0x1p-12f ? x : one(k, s);
	*cosp = one(k + 16, s);
}
