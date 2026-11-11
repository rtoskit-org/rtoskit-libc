/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanhf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast tanhf of the single-precision FPU cores (PLAN D162, D177): below 1 m/(m + 2) with m = e^(2|x|) - 1 as a pair
 * (__libc_fast_expm1f_pair, k <= 3), the pairs divided (__libc_fast_divide_pairs); up to 9.1 1 - 2/(E + 1) with E = e^(2|x|)
 * as a pair, 2/(E + 1) at most 0.24; beyond, +-1. Below 1.5 ulp to nearest (the trigonometric note, section 10); float
 * only, round to nearest only as the layer (D57). C23 F.10.2.6: +-0 itself, +-infinity +-1.
 */
float tanhf(float x)
{
	float a = __builtin_fabsf(x);
	float t;
	float kf;
	float u_hi;
	float u_lo;
	float hi;
	float lo;
	float d;
	float d_lo;
	float q;
	float q_lo;
	float error;
	int k;

	if (isnan(x))
		return x + x;
	/* x^3/3 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	if (isinf(x))
		return x < 0 ? -1.0f : 1.0f;
	/* From 9.01 tanh is within a quarter ulp of +-1: +-1, inexact (a division at run time, not a folded constant) */
	if (a > 9.1f) {
		hi = 1.0f - 0x1p-30f / a;
		return x < 0 ? -hi : hi;
	}
	t = 2.0f * a;
	if (a < 1.0f) {
		kf = __builtin_fmaf(t, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;
		u_hi = __builtin_fmaf(-kf, 0x1.62e430p-1f, t);
		u_lo = kf * 0x1.05c610p-29f;
		hi = __libc_fast_expm1f_pair(u_hi, u_lo, (int)kf, __libc_fast_expf_q3(u_hi + u_lo), &lo);
		d = __libc_fast_two_sum(hi, 2.0f, &error);
		hi = __libc_fast_divide_pairs(hi, lo, d, error + lo);
		return x < 0 ? -hi : hi;
	}
	/* E = 2^k P exactly scaled (k <= 27), E + 1 by a two-sum, 2/(E + 1) with its remainder, 1 minus it by a two-sum */
	hi = __libc_fast_expf_pair(t, &k, &lo);
	hi *= __libc_float_from((uint32_t)(k + 127) << 23);
	lo *= __libc_float_from((uint32_t)(k + 127) << 23);
	d = __libc_fast_two_sum(hi, 1.0f, &error);
	d_lo = error + lo;
	q = 2.0f / d;
	q_lo = (__builtin_fmaf(-q, d, 2.0f) - q * d_lo) / d;
	hi = __libc_fast_two_sum(1.0f, -q, &error);
	hi = hi + (error - q_lo);

	return x < 0 ? -hi : hi;
}
