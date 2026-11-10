/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expm1f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast expm1f of the single-precision FPU cores (PLAN D162, D171): x = k ln2 + u with u_hi = x - k ln2f exact
 * by a fused multiply-add and u_lo = k (ln2f - ln2), as expf; e^u 2^k - 1 by __libc_fast_expm1f_kernel, which keeps
 * the relative accuracy near 0 (k = 0: u_hi is x). Below 1 ulp to nearest (the note, section 12); float only, round
 * to nearest only as the layer (D57).
 */
float expm1f(float x)
{
	float k;

	if (isnan(x))
		return x + x;
	if (x > 89.0f)
		return x * 0x1p127f;
	/* e^x is below a quarter of -1's ulp below 1: -1 */
	if (x < -18.0f)
		return -1.0f + 0x1p-100f;
	/* x^2/2 is below half an ulp of x (and may underflow) */
	if (__builtin_fabsf(x) < 0x1p-25f)
		return x;

	k = __builtin_fmaf(x, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;

	return __libc_fast_expm1f_kernel(__builtin_fmaf(-k, 0x1.62e430p-1f, x), k * 0x1.05c610p-29f, (int)k);
}
