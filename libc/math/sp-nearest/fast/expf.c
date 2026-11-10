/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast expf of the single-precision FPU cores (PLAN D162, D168): x = k ln2 + u in float, with k = round(x / ln2)
 * and u = x - k ln2f exact by the fused multiply-add (the product's 32 bits fit, the difference is a multiple of
 * 2^-24 below 2^-1.5), the rest of k ln2 in u_lo; e^u by __libc_fast_expf_kernel. Below 1 ulp to nearest (the
 * note, section 4). The cores of this layer round to nearest only (D57).
 */
float expf(float x)
{
	float k;

	if (isnan(x))
		return x + x;
	if (x > 89.0f)
		return x * 0x1p127f;
	if (x < -104.0f)
		return isinf(x) ? 0.0f : (__libc_math_raise_underflow(), 0.0f);

	/* round(x / ln2) by the 1.5 * 2^23 shift (|x / ln2| < 2^22) */
	k = __builtin_fmaf(x, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;

	return __libc_fast_expf_kernel(__builtin_fmaf(-k, 0x1.62e430p-1f, x), k * 0x1.05c610p-29f, (int)k);
}
