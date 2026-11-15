/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp_special.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The slow path of the fast exp (libc/math/fast/exp.c and libc/math/dp-fixed/fast/exp.c, PLAN D187, D188): tiny
 * arguments, the ends, the infinities and NaN; between the common path's edge and the ends, the common path's own
 * reduction and polynomial with __libc_fast_exp_scale's two-step scaling (P11.06h, D211 D), no strict core
 */
double __libc_fast_exp_special(double x)
{
	double kd;
	int32_t k;
	double u;

	if (isnan(x))
		return x + x;
	/* e^x rounds as 1 + x, with inexact unless x is 0 */
	if (__builtin_fabs(x) < 0x1p-54)
		return 1.0 + x;
	if (x > 0x1.62e42fefa39efp+9)
		return isinf(x) ? x : __libc_math_overflow(1.0);
	/* Below ln 2^-1075 the result rounds to zero (2^-1074 upward), with underflow; e^-inf is +0 exactly */
	if (x < -0x1.74910d52d3051p+9)
		return isinf(x) ? 0.0 : __libc_math_underflow(1.0);
	/* From 512 to the ends: the common path's reduction and polynomial, scaled in two steps (P11.06h) */
	k = __libc_fast_exp_index(x * 0x1.71547652b82fep+6, &kd);
	u = (x - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39;
	return __libc_fast_exp_scale(k, __libc_fast_expm1_poly(u));
}
