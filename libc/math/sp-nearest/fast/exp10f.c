/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp10f of the single-precision FPU cores (PLAN D162, D168): x = k log10(2) + r with k = round(x log2 10)
 * and r = x - k log10(2)f exact by the fused multiply-add, the rest of k log10(2) in r_lo; u = (r + r_lo) ln10 as
 * u_hi + u_lo, e^u by __libc_fast_expf_kernel. Below 1 ulp to nearest (the note, section 4).
 */
float exp10f(float x)
{
	float k;
	float r;
	float r_lo;
	float u;

	if (isnan(x))
		return x + x;
	if (x > 39.0f)
		return x * 0x1p127f;
	if (x < -46.0f)
		return isinf(x) ? 0.0f : (__libc_math_raise_underflow(), 0.0f);

	k = __builtin_fmaf(x, 0x1.a934f0p+1f, 0x1.8p23f) - 0x1.8p23f;
	r = __builtin_fmaf(-k, 0x1.344136p-2f, x);
	r_lo = k * 0x1.ec10c0p-27f;
	u = r * 0x1.26bb1cp+1f;

	return __libc_fast_expf_kernel(u, __builtin_fmaf(r, 0x1.26bb1cp+1f, -u) + __builtin_fmaf(r, -0x1.12aabap-25f, r_lo * 0x1.26bb1cp+1f), (int)k);
}
