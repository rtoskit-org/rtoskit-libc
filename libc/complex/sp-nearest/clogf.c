/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * clogf.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <complex.h>
#include <math.h>

#include "internal/complex.h"
#include "internal/math.h"

/*
 * C23 7.3.7.2, G.6.3.2: log|z| + i arg z, with log|z| = log2(a^2 + b^2) ln 2 / 2
 * on the parts scaled by 2^-scale, plus scale ln 2, the squares exact as
 * float-float (__libc_clogf_modulus, shared with the inverse functions). Near |z| = 1 it is log1p(a^2 + b^2 - 1)/2 with the 1 taken
 * out exactly, so a modulus near 1 keeps its relative accuracy; for a = 1
 * and b below 2^-20 it is b^2/2 from the square of b scaled into [1, 2),
 * which would underflow unscaled (PLAN D65).
 */
float complex clogf(float complex z)
{
	float x = __libc_ref(z);
	float y = __libc_imf(z);
	float a = fmaxf(fabsf(x), fabsf(y));
	float b = fminf(fabsf(x), fabsf(y));
	struct __libc_ff square_a;
	struct __libc_ff square_b;
	struct __libc_ff value;
	int scale;

	if (isinf(x) || isinf(y))
		return CMPLXF(INFINITY, __libc_atan2f_accurate(y, x));
	if (isnan(x) || isnan(y))
		return CMPLXF(x + y, x + y);
	if (a == 0)
		return CMPLXF(-1.0f / a, __libc_atan2f_accurate(y, x));

	if (a == 1.0f && b < 0x1p-20f) {
		scale = ilogbf(b);
		square_b = __libc_ff_product(scalbnf(b, -scale), scalbnf(b, -scale));
		return CMPLXF(__libc_exp2f_scale(square_b, 2 * scale - 1), __libc_atan2f_accurate(y, x));
	}
	if (a >= 0x1.6a09e6p-1f && a <= 2.0f) {
		/* a^2 - 1 is exact (Sterbenz), then the four parts are summed in float-float */
		square_a = __libc_ff_product(a, a);
		square_b = __libc_ff_product(b, b);
		value = __libc_ff_add(__libc_ff_sum(square_a.hi - 1.0f, square_b.hi), __libc_ff_sum(square_a.lo, square_b.lo));
		if (fabsf(value.hi) < 0x1p-6f) {
			value = __libc_ff_log1p_series(value);
		} else {
			value = __libc_ff_add((struct __libc_ff){ .hi = 1.0f, .lo = 0.0f }, value);
			value = __libc_ff_multiply(__libc_log2f_core(value.hi, value.lo), __LIBC_LN2F);
		}
		return CMPLXF(0.5f * value.hi + 0.5f * value.lo, __libc_atan2f_accurate(y, x));
	}

	value = __libc_clogf_modulus(x, y);

	return CMPLXF(value.hi + value.lo, __libc_atan2f_accurate(y, x));
}
