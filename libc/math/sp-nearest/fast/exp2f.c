/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp2f of the single-precision FPU cores (PLAN D162, D168): x = k + r with k = round(x) and r exact,
 * u = r ln2 as u_hi + u_lo (the product's rounding error by a fused multiply-add, and ln2's low part), e^u by
 * __libc_fast_expf_kernel. Below 1 ulp to nearest and exact at the integers (the note, section 4).
 */
float exp2f(float x)
{
	float k;
	float r;
	float u;

	if (isnan(x))
		return x + x;
	if (x >= 128.0f)
		return x * 0x1p127f;
	if (x < -151.0f)
		return isinf(x) ? 0.0f : (__libc_math_raise_underflow(), 0.0f);

	k = (x + 0x1.8p23f) - 0x1.8p23f;
	r = x - k;
	u = r * 0x1.62e430p-1f;

	return __libc_fast_expf_kernel(u, __builtin_fmaf(r, 0x1.62e430p-1f, -u) - r * 0x1.05c610p-29f, (int)k);
}
