/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * powf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>

#include "internal/math-fast.h"

/*
 * The fast powf of the single-precision FPU cores (PLAN D162, D170): the special cases of C23 F.10.4.5
 * (__libc_fast_powf_special), then 2^t with t = y log2 |x| carried as hi + lo: log2 |x| to about 2^-35
 * (__libc_fast_log2f_extended), the product's error by a fused multiply-add, t = k + f + t_lo with f exact, and
 * e^((f + t_lo) ln2) 2^k by __libc_fast_expf_kernel. Below 1 ulp to nearest (the note, section 10); float only,
 * round to nearest only as the layer (D57).
 */
float powf(float x, float y)
{
	float result;
	bool negative;
	float l_hi;
	float l_lo;
	float t_hi;
	float t_lo;
	float k;
	float f;
	float u;

	if (__libc_fast_powf_special(x, y, &result, &negative))
		return result;

	l_hi = __libc_fast_log2f_extended(fabsf(x), &l_lo);
	t_hi = y * l_hi;
	/* Beyond 2^128 or below half the smallest subnormal, whatever the low parts (|l_lo| < 2^-20 |l_hi|) */
	if (!(t_hi < 128.5f)) {
		__libc_math_raise_overflow();
		return negative ? -INFINITY : INFINITY;
	}
	if (!(t_hi > -151.0f)) {
		__libc_math_raise_underflow();
		return negative ? -0.0f : 0.0f;
	}
	t_lo = __builtin_fmaf(y, l_hi, -t_hi) + y * l_lo;

	k = (t_hi + 0x1.8p23f) - 0x1.8p23f;
	f = t_hi - k;
	u = f * 0x1.62e430p-1f;
	result = __libc_fast_expf_kernel(u, __builtin_fmaf(f, 0x1.62e430p-1f, -u) + __builtin_fmaf(f, -0x1.05c610p-29f, t_lo * 0x1.62e430p-1f), (int)k);

	return negative ? -result : result;
}
