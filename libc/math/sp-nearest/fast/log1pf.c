/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log1pf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast log1pf of the single-precision FPU cores (PLAN D162, D171): 1 + x reduced against a table entry c_i with
 * the rounding of 1 + x carried (__libc_fast_log1pf_reduce; r = x itself within 2^-5 of 0), then as logf: k ln2_hi + log c_i's high part exact, the last addition the only large rounding.
 * Below 1 ulp to nearest (the note, section 12); float only, round to nearest only as the layer (D57).
 */
float log1pf(float x)
{
	const float *entry;
	float high;
	float low;
	float kf;
	float r;
	float lo;
	int k;
	int i;

	if (isnan(x))
		return x + x;
	if (x < -1.0f)
		return __libc_math_invalidf();
	/* log(1 + -1) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == -1.0f)
		return -1.0f / fabsf(x + 1.0f);
	if (isinf(x))
		return x;
	/* x^2/2 is below half an ulp of x (and may underflow) */
	if (__builtin_fabsf(x) < 0x1p-25f)
		return x;

	r = __libc_fast_log1pf_reduce(x, &k, &i, &lo);
	entry = __libc_fast_logf_ln[i];
	kf = (float)k;
	high = __builtin_fmaf(kf, 0x1.62e400p-1f, entry[0]);
	low = __builtin_fmaf(kf, 0x1.7f7d1cp-20f, entry[1]);

	return high + (r + (low + (__libc_fast_log1pf_tail(r) + lo)));
}
