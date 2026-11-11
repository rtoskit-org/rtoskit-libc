/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asinhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast asinhf of the single-precision FPU cores (PLAN D162, D177): up to 2^12 log1p(|x| + sqrt(1 + x^2) - 1) with
 * 1 + x^2 and its square root as pairs, so the argument is accurate relative to |x| (__libc_fast_log1pf_pair); beyond,
 * log 2|x| (the next term, 1/(4x^2), below 2^-26). Below 1.5 ulp to nearest (the trigonometric note, section 10); float
 * only, round to nearest only as the layer (D57). C23 F.10.2.2: +-0 and +-infinity themselves.
 */
float asinhf(float x)
{
	float a = __builtin_fabsf(x);
	float z;
	float z_lo;
	float w;
	float w_lo;
	float s;
	float s_lo;
	float t;
	float t_lo;
	float u;
	float u_lo;
	float error;
	float r;

	if (isnan(x) || isinf(x))
		return x + x;
	/* x^3/6 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	if (a > 0x1p12f) {
		r = __libc_fast_logf_twice(a);
		return x < 0 ? -r : r;
	}
	/* z = x^2 exactly as a pair, w = 1 + z, s = sqrt(w), t = s - 1 (exact up to s = 2, else by a two-sum), u = |x| + t */
	z = a * a;
	z_lo = __builtin_fmaf(a, a, -z);
	w = __libc_fast_two_sum(1.0f, z, &error);
	w_lo = error + z_lo;
	s = __libc_fast_sqrtf_pair(w, w_lo, &s_lo);
	t = __libc_fast_two_sum(s, -1.0f, &error);
	t_lo = error + s_lo;
	u = __libc_fast_two_sum(a, t, &error);
	u_lo = error + t_lo;
	r = __libc_fast_log1pf_pair(u, u_lo);

	return x < 0 ? -r : r;
}
