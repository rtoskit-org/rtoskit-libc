/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_acoshf_accurate.c
 *
 * Created on: Nov 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"

/*
 * The float complex layer's acoshf in the fast flavour of the single-precision FPU cores (PLAN D65, D236, D238): the
 * strict layer's method (D62), as the public fast acoshf has a 1.5-ulp bound and the complex parts must stay within
 * 0.52. C23 7.12.5.1: up to 2^20, log1p((x - 1) + sqrt((x - 1)(x + 1))) with everything as float-float, so that the
 * result keeps its relative accuracy near 1; beyond, log(2x).
 */
float __libc_acoshf_accurate(float x)
{
	struct __libc_ff value;
	struct __libc_ff below;

	if (isnan(x))
		return x + x;
	if (x < 1.0f)
		return __libc_math_invalidf();
	if (x == 1.0f)
		return 0.0f;
	if (isinf(x))
		return x;

	if (x > 0x1p20f) {
		value = __libc_ff_add(__libc_log2f_core(x, 0.0f), (struct __libc_ff){ .hi = 1.0f, .lo = 0.0f });
	} else {
		below = __libc_ff_sum(x, -1.0f);
		value = __libc_ff_add(below, __libc_ff_sqrt(__libc_ff_multiply(below, __libc_ff_sum(x, 1.0f))));
		value = __libc_ff_add((struct __libc_ff){ .hi = 1.0f, .lo = 0.0f }, value);
		value = __libc_log2f_core(value.hi, value.lo);
	}
	value = __libc_ff_multiply(value, __LIBC_LN2F);

	return value.hi + value.lo;
}
