/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * logf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast logf of the single-precision FPU cores (PLAN D162, D168): x = 2^k z reduced against a table entry c_i
 * (__libc_fast_logf_reduce), k ln2 + log c_i + log1p(r): k ln2_hi + log c_i's high part is exact (both multiples of 2^-16, the sum below 2^7); so the last addition is the only large rounding. Below 1 ulp to nearest
 * (the note, section 8); float only, round to nearest only as the layer (D57).
 */
float logf(float x)
{
	const float *entry;
	float high;
	float low;
	float kf;
	float r;
	int k;
	int i;

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log(+-0) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	r = __libc_fast_logf_reduce(x, &k, &i);
	entry = __libc_fast_logf_ln[i];
	kf = (float)k;
	high = __builtin_fmaf(kf, 0x1.62e400p-1f, entry[0]);
	low = __builtin_fmaf(kf, 0x1.7f7d1cp-20f, entry[1]);

	return high + (r + (low + __libc_fast_log1pf_tail(r)));
}
