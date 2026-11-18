/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * casinhf.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <complex.h>
#include <math.h>

#include "internal/complex.h"
#include "internal/math.h"

/*
 * C23 7.3.6.2, G.6.2.2: special cases first; then casinh(z) = -i asin(iz)
 * with Kahan's asin in float-float: for w = iz, u = sqrt(1 - w),
 * v = sqrt(1 + w), asin(w) = atan2(Re w, Re(u v)) + i asinh(Im(conj(u) v)).
 * From |z| = 2^24 the asymptotic form copysign(log 2|z|, x) + i atan2(y, |x|)
 * is good to 2^-48. For |x| below 2^-60 (and |y| not 1) the first-order forms
 * x/sqrt(1 - y^2) + i asin y and copysign(acosh |y|, x) + i copysign(pi/2, y)
 * are exact to 2^-120; for |y| below 2^-60 the imaginary part is
 * y/Re sqrt(1 + z^2), so a tiny part keeps its relative accuracy (PLAN D67).
 */
float complex casinhf(float complex z)
{
	float x = __libc_ref(z);
	float y = __libc_imf(z);
	struct __libc_ff u_real;
	struct __libc_ff u_imag;
	struct __libc_ff v_real;
	struct __libc_ff v_imag;
	struct __libc_ff value;
	float imag;

	if (isinf(x) || isinf(y)) {
		if (isnan(x))
			return CMPLXF(y, x);
		if (isnan(y))
			return CMPLXF(x, y);
		return CMPLXF(copysignf(INFINITY, x), isinf(y) ? copysignf(isinf(x) ? 0x1.921fb6p-1f : 0x1.921fb6p+0f, y) : copysignf(0.0f, y));
	}
	if (isnan(x))
		return CMPLXF(x, y == 0 ? y : x);
	if (isnan(y))
		return CMPLXF(y, y);

	if (fmaxf(fabsf(x), fabsf(y)) >= 0x1p24f) {
		value = __libc_ff_add(__libc_clogf_modulus(x, y), __LIBC_LN2F);
		return CMPLXF(copysignf(value.hi + value.lo, x), __libc_atan2f_accurate(y, fabsf(x)));
	}

	if (x == 0 && fabsf(y) == 1.0f)
		return CMPLXF(x, copysignf(0x1.921fb6p+0f, y));
	if (fabsf(x) < 0x1p-60f && fabsf(y) != 1.0f) {
		value = __libc_ff_multiply(__libc_ff_sum(1.0f, -fabsf(y)), __libc_ff_sum(1.0f, fabsf(y)));
		if (fabsf(y) < 1.0f) {
			value = __libc_ff_divide((struct __libc_ff){ .hi = scalbnf(x, 64), .lo = 0.0f }, __libc_ff_sqrt(value));
			return CMPLXF(x == 0 ? x : __libc_exp2f_scale(value, -64), __libc_asinf_accurate(y));
		}
		return CMPLXF(copysignf(acoshf(fabsf(y)), x), copysignf(0x1.921fb6p+0f, y));
	}
	/* w = iz = -y + ix */
	__libc_csqrtf_core(__libc_ff_sum(1.0f, y), -x, &u_real, &u_imag);
	__libc_csqrtf_core(__libc_ff_sum(1.0f, -y), x, &v_real, &v_imag);
	value = __libc_ff_add(__libc_ff_multiply(u_real, v_real), __libc_ff_multiply((struct __libc_ff){ .hi = -u_imag.hi, .lo = -u_imag.lo }, v_imag));
	if (fabsf(y) < 0x1p-60f) {
		/* atan2(y, X) = y/X for X = Re sqrt(1 + z^2) >= 1 */
		imag = y == 0 ? y : __libc_exp2f_scale(__libc_ff_divide((struct __libc_ff){ .hi = scalbnf(y, 64), .lo = 0.0f }, value), -64);
	} else {
		value = __libc_atan2f_core((struct __libc_ff){ .hi = -y, .lo = 0.0f }, value);
		imag = -(value.hi + value.lo);
	}
	value = __libc_ff_add(__libc_ff_multiply(u_real, v_imag), __libc_ff_multiply((struct __libc_ff){ .hi = -u_imag.hi, .lo = -u_imag.lo }, v_real));
	value = __libc_asinhf_core(value);

	return CMPLXF(__libc_ff_round_product(value, (struct __libc_ff){ .hi = 1.0f, .lo = 0.0f }, 0), imag);
}
