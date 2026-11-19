/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * acoshf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast acoshf of the single-precision FPU cores (PLAN D162, D177, D238): below 2 log1p(t + sqrt(t (t + 2))) with t = x - 1
 * (exact), t (t + 2) and its square root as pairs (__libc_fast_log1pf_pair); up to 2^12 log(x + sqrt(x^2 - 1)) on the
 * pair (__libc_fast_logf_pair); beyond, log 2x (the next term, -1/(4x^2),
 * below 2^-26). Within the 1.5-ulp bound to nearest (the float trigonometric note, sections 10 and 29); float only, round to nearest only as the
 * layer (D57). C23 F.10.2.1: below 1 invalid, acosh 1 = +0, +infinity itself.
 */
float acoshf(float x)
{
	float t;
	float v;
	float v_lo;
	float w;
	float w_lo;
	float s;
	float s_lo;
	float u;
	float u_lo;
	float error;

	if (isnan(x))
		return x + x;
	if (x < 1.0f)
		return __libc_math_invalidf();
	if (x == 1.0f)
		return 0.0f;
	if (isinf(x))
		return x;
	if (x > 0x1p12f)
		return __libc_fast_logf_twice(x);
	if (x >= 2.0f) {
		/* log(x + sqrt(x^2 - 1)): x^2 as an exact pair, x^2 - 1 and x + s by Fast2Sum (x^2 >= 4, x > s) */
		w = x * x;
		w_lo = ((w - (w - 1.0f)) - 1.0f) + __builtin_fmaf(x, x, -w);
		w -= 1.0f;
		s = __libc_fast_sqrtf_pair(w, w_lo, &s_lo);
		u = x + s;
		u_lo = (s - (u - x)) + s_lo;
		return __libc_fast_logf_pair(u, u_lo);
	}
	/* t + 2 by a two-sum, t (t + 2) with its product error by a fused multiply-add */
	t = x - 1.0f;
	v = __libc_fast_two_sum(t, 2.0f, &v_lo);
	w = t * v;
	w_lo = __builtin_fmaf(t, v, -w) + t * v_lo;
	s = __libc_fast_sqrtf_pair(w, w_lo, &s_lo);
	u = __libc_fast_two_sum(t, s, &error);
	u_lo = error + s_lo;

	return __libc_fast_log1pf_pair(u, u_lo);
}
