/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2f.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"

/*
 * C23 7.12.4.4: the special cases of F.10.1.4 directly; far apart with a
 * positive x the result is y/x rounded once (subnormal results
 * included); otherwise the atan2 core on operands scaled to the larger one
 * (PLAN D61).
 */
float atan2f(float y, float x)
{
	struct __libc_ff value;
	int scale;
	int shift;

	if (isnan(x) || isnan(y))
		return x + y;
	if (y == 0)
		return signbit(x) ? copysignf(0x1.921fb6p1f, y) : y;
	if (x == 0)
		return copysignf(0x1.921fb6p0f, y);
	if (isinf(y))
		return copysignf(isinf(x) ? (x > 0 ? 0x1.921fb6p-1f : 0x1.2d97c8p1f) : 0x1.921fb6p0f, y);
	if (isinf(x))
		return x > 0 ? copysignf(0.0f, y) : copysignf(0x1.921fb6p1f, y);
	/* Beyond 2^40 apart the result is +-pi/2 or +-pi to far below half an ulp */
	if (ilogbf(y) - ilogbf(x) > 40)
		return copysignf(0x1.921fb6p0f, y);
	if (x < 0 && ilogbf(y) - ilogbf(x) < -40)
		return copysignf(0x1.921fb6p1f, y);

	if (x > 0 && ilogbf(y) - ilogbf(x) < -40) {
		/* atan(q) = q - q^3/3 with q below 2^-40: q rounded once */
		shift = ilogbf(y) - ilogbf(x);
		value = __libc_ff_divide((struct __libc_ff){ .hi = scalbnf(y, -ilogbf(y)), .lo = 0.0f }, (struct __libc_ff){ .hi = scalbnf(x, -ilogbf(x)), .lo = 0.0f });
		value = __libc_ff_multiply(value, (struct __libc_ff){ .hi = 1.0f, .lo = 0.0f });
		return __libc_exp2f_scale(value, shift);
	}

	scale = -ilogbf(fabsf(x) > fabsf(y) ? x : y);
	value = __libc_atan2f_core((struct __libc_ff){ .hi = scalbnf(y, scale), .lo = 0.0f }, (struct __libc_ff){ .hi = scalbnf(x, scale), .lo = 0.0f });

	return value.hi + value.lo;
}

/* The float complex layer's accurate atan2f (D236): this one, within 0.52 ulp */
float __libc_atan2f_accurate(float y, float x) __attribute__((__alias__("atan2f")));
