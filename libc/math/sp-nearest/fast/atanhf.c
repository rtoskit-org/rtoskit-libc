/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atanhf of the single-precision FPU cores (PLAN D162, D177): log1p(2|x|/(1 - |x|))/2 with 1 - |x| by a two-sum and
 * the quotient's remainder by a fused multiply-add (__libc_fast_log1pf_pair), the sign of x. Below 1.5 ulp to nearest (the
 * trigonometric note, section 10); float only, round to nearest only as the layer (D57). C23 F.10.2.3: +-1 gives
 * +-infinity with divide-by-zero, beyond invalid, +-0 itself.
 */
float atanhf(float x)
{
	float a = __builtin_fabsf(x);
	float d;
	float d_lo;
	float q;
	float q_lo;
	float r;

	if (isnan(x))
		return x + x;
	if (a > 1.0f)
		return __libc_math_invalidf();
	/* +-infinity with divide-by-zero: a division at run time, not a folded constant */
	if (a == 1.0f)
		return x / (a - 1.0f);
	/* x^3/3 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	d = __libc_fast_two_sum(1.0f, -a, &d_lo);
	q = 2.0f * a / d;
	q_lo = (__builtin_fmaf(-q, d, 2.0f * a) - q * d_lo) / d;
	r = 0.5f * __libc_fast_log1pf_pair(q, q_lo);

	return x < 0 ? -r : r;
}
