/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cacosf.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <complex.h>
#include <math.h>

#include "internal/complex.h"
#include "internal/math.h"

/*
 * C23 7.3.5.1, G.6.1.1: Kahan's formula with u = sqrt(1 - z) and
 * v = sqrt(1 + z) in float-float: cacos(z) = 2 atan2(Re u, Re v) +
 * i asinh(Re v Im u - Im v Re u) (the two products have the same sign, so
 * the difference does not cancel), 1 -+ x exact as float-float. From |z| =
 * 2^24 the asymptotic form atan2(|y|, x) - i copysign(log 2|z|, y) is good to
 * 2^-48. For |y| below 2^-60 (and |x| not 1) the first-order forms
 * acos x - i y/sqrt(1 - x^2) and |y|/sqrt(x^2 - 1) - i copysign(acosh |x|, y)
 * (pi less that for x below -1) are exact to 2^-120, and keep a tiny y's
 * relative accuracy. Special cases first (PLAN D67).
 */
float complex cacosf(float complex z)
{
	float x = __libc_ref(z);
	float y = __libc_imf(z);
	float real;
	struct __libc_ff u_real;
	struct __libc_ff u_imag;
	struct __libc_ff v_real;
	struct __libc_ff v_imag;
	struct __libc_ff value;

	if (isinf(x) || isinf(y)) {
		if (isnan(x))
			return CMPLXF(x, -y);
		if (isnan(y))
			return CMPLXF(y, -x);
		if (isinf(x))
			real = x > 0 ? (isinf(y) ? 0x1.921fb6p-1f : 0.0f) : (isinf(y) ? 0x1.2d97c8p+1f : 0x1.921fb6p+1f);
		else
			real = 0x1.921fb6p+0f;
		return CMPLXF(real, -copysignf(INFINITY, y));
	}
	if (isnan(x))
		return CMPLXF(x, x);
	if (isnan(y))
		return CMPLXF(x == 0 ? 0x1.921fb6p+0f : y, y);

	if (fmaxf(fabsf(x), fabsf(y)) >= 0x1p24f) {
		value = __libc_ff_add(__libc_clogf_modulus(x, y), __LIBC_LN2F);
		return CMPLXF(__libc_atan2f_accurate(fabsf(y), x), -copysignf(value.hi + value.lo, y));
	}

	if (y == 0 && fabsf(x) == 1.0f)
		return CMPLXF(x > 0 ? 0.0f : 0x1.921fb6p+1f, -y);
	if (fabsf(y) < 0x1p-60f && fabsf(x) != 1.0f) {
		value = __libc_ff_multiply(__libc_ff_sum(1.0f, -fabsf(x)), __libc_ff_sum(1.0f, fabsf(x)));
		if (fabsf(x) < 1.0f) {
			value = __libc_ff_divide((struct __libc_ff){ .hi = scalbnf(-y, 64), .lo = 0.0f }, __libc_ff_sqrt(value));
			return CMPLXF(__libc_acosf_accurate(x), y == 0 ? -y : __libc_exp2f_scale(value, -64));
		}
		value = __libc_ff_divide((struct __libc_ff){ .hi = scalbnf(fabsf(y), 64), .lo = 0.0f }, __libc_ff_sqrt((struct __libc_ff){ .hi = -value.hi, .lo = -value.lo }));
		real = x > 0 ? (y == 0 ? 0.0f : __libc_exp2f_scale(value, -64)) : 0x1.921fb6p+1f;
		return CMPLXF(real, -copysignf(__libc_acoshf_accurate(fabsf(x)), y));
	}
	__libc_csqrtf_core(__libc_ff_sum(1.0f, -x), -y, &u_real, &u_imag);
	__libc_csqrtf_core(__libc_ff_sum(1.0f, x), y, &v_real, &v_imag);
	value = __libc_atan2f_core(u_real, v_real);
	real = 2.0f * value.hi + 2.0f * value.lo;
	value = __libc_ff_add(__libc_ff_multiply(v_real, u_imag), __libc_ff_multiply((struct __libc_ff){ .hi = -v_imag.hi, .lo = -v_imag.lo }, u_real));
	value = __libc_asinhf_core(value);

	return CMPLXF(real, __libc_ff_round_product(value, (struct __libc_ff){ .hi = 1.0f, .lo = 0.0f }, 0));
}
