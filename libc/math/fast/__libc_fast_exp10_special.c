/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp10_special.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The slow path of the fast exp10 (libc/math/fast/exp10.c and libc/math/dp-fixed/fast/exp10.c, PLAN D187, D188): tiny
 * arguments, the ends, the infinities and NaN; between the common path's edge and the ends, the common path's own
 * reduction and polynomial with __libc_fast_exp_scale's two-step scaling (P11.06h, D211 D), no strict core
 */
double __libc_fast_exp10_special(double x)
{
	double kd;
	int32_t k;
	double r;

	if (isnan(x))
		return x + x;
	/* 10^x rounds as 1 + x ln 10 (1 + x alone was 1.15 ulp below 1, measured) */
	if (__builtin_fabs(x) < 0x1p-54)
		return 1.0 + x * 0x1.26bb1bbb55516p+1;
	if (x > 0x1.34413509f79fep+8)
		return isinf(x) ? x : __libc_math_overflow(1.0);
	if (x < -0x1.439b746e36b52p+8)
		return isinf(x) ? 0.0 : __libc_math_underflow(1.0);
	/* From 256 to the ends: the common path's reduction (log10 2 / 64 in two parts) and polynomial, scaled in two steps */
	k = __libc_fast_exp_index(x * 0x1.a934f0979a371p+7, &kd);
	r = (x - kd * 0x1.3441350800000p-8) - kd * 0x1.f79fef311f12bp-40;
	return __libc_fast_exp_scale(k, __libc_fast_expm1_poly(r * 0x1.26bb1bbb55516p+1));
}
