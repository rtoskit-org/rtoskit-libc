/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cosf.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"
#include "libm-fixed.h"
#include "trig-fixed.h"

/*
 * C23 7.12.4.5: x reduced exactly to k pi/32 + s (PLAN D60), then the fixed-point
 * core; the float-float core takes the results the fixed-point one cannot
 * round with certainty (PLAN D86).
 */
float cosf(float x)
{
	struct __libc_ff s;
	struct __libc_ff value;
	bool negative;
	int exponent;
	uint64_t v;
	uint32_t result;
	int k;
	struct __libc_trig_fixed fast;
	double wide;
	float rounded;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();

	/* The D136 kernel, rounded to float (P9.02, Q41); the D87 path below for what it declines */
	if (__libc_trig_fixed_from_float(x, &wide) && __libc_trig_fixed(wide, __LIBC_TRIG_FIXED_COS, nullptr, &fast) && __libc_trig_fixed_roundf(&fast, &rounded))
		return rounded;

	if (fabsf(x) < 0x1p-6f) {
		k = 0;
		s = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	} else {
		k = __libc_rem_pio2f(x, &s);
	}

	v = __libc_sinf_fixed(k + 16, s.hi, s.lo, &exponent, &negative);
	if (v != 0 && __libc_fixed_round((uint32_t)(v >> 31), exponent, __LIBC_FIXED_SIN_ERROR, &result))
		return __libc_float_from(result | (negative ? 0x80000000u : 0));

	value = __libc_sinf_core(k + 16, s);

	return value.hi + value.lo;
}
