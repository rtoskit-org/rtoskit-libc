/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanf.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"
#include "trig-fixed.h"

/*
 * C23 7.12.4.7: sin over cos of the reduced argument (PLAN D60), both from
 * the fixed-point core, divided to 33 bits. The float-float quotient takes
 * the results the fixed-point one cannot round with certainty (PLAN D86).
 */
float tanf(float x)
{
	struct __libc_ff s;
	struct __libc_ff value;
	bool sine_negative;
	bool cosine_negative;
	int sine_exponent;
	int cosine_exponent;
	uint64_t sine;
	uint64_t cosine;
	uint32_t result;
	int k;
	struct __libc_trig_fixed fast;
	double wide;
	float rounded;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return __libc_math_invalidf();
	/* The x^3 term is below half an ulp of x: the result rounds to x (keeping -0) */
	if (fabsf(x) < 0x1p-13f)
		return x;

	/* sin x / cos x from the D136 kernel, rounded to float (P9.03, Q42); the D87 path below for what it declines */
	if (__libc_trig_fixed_from_float(x, &wide) && __libc_tan_fixed(wide, &fast) && __libc_trig_fixed_roundf(&fast, __LIBC_TAN_FIXED_ERROR, &rounded))
		return rounded;

	if (fabsf(x) < 0x1p-6f) {
		k = 0;
		s = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	} else {
		k = __libc_rem_pio2f(x, &s);
	}

	sine = __libc_sinf_fixed(k, s.hi, s.lo, &sine_exponent, &sine_negative);
	cosine = __libc_sinf_fixed(k + 16, s.hi, s.lo, &cosine_exponent, &cosine_negative);
	if (sine != 0 && cosine != 0) {
		/*
		 * q = sine 2^32 / cosine in [2^32, 2^33), doubled when sine < cosine: the
		 * quotient by the cosine's top 32 bits (and one more bit from the remainder),
		 * less q times the cosine's low 32 bits over the cosine, at most 3
		 */
		uint32_t top = (uint32_t)(cosine >> 32);
		uint32_t low = (uint32_t)cosine;
		uint64_t quotient = sine / top;
		uint64_t remainder = sine - quotient * top;
		uint64_t correction;
		int exponent = sine_exponent - cosine_exponent;

		if ((quotient >> 32) == 0) {
			quotient = (quotient << 1) | ((remainder << 1) >= top);
			--exponent;
		}
		correction = low + (uint64_t)__libc_fixed_high((uint32_t)quotient, low);
		while (correction >= top) {
			correction -= top;
			--quotient;
		}

		if ((quotient >> 32) != 0 && __libc_fixed_round((uint32_t)quotient, exponent, __LIBC_FIXED_TAN_ERROR, &result))
			return __libc_float_from(result | (sine_negative != cosine_negative ? 0x80000000u : 0));
	}

	value = __libc_ff_divide(__libc_sinf_core(k, s), __libc_sinf_core(k + 16, s));

	return value.hi + value.lo;
}
