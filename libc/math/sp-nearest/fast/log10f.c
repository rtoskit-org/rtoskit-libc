/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast log10f of the single-precision FPU cores (PLAN D162, D168): x = 2^k z reduced against a table entry c_i
 * (__libc_fast_logf_reduce), k log10(2) + log10 c_i + log1p(r) / ln10: the high parts' sum is exact; r / ln10 carries its rounding error by a fused multiply-add; so the last addition is the only large rounding. Below 1 ulp to nearest
 * (the note, section 8); float only, round to nearest only as the layer (D57).
 */
float log10f(float x)
{
	const float *entry;
	float high;
	float low;
	float kf;
	float r;
	float scaled;
	int k;
	int i;

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log10(+-0) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	r = __libc_fast_logf_reduce(x, &k, &i);
	entry = __libc_fast_logf_log10[i];
	kf = (float)k;
	high = __builtin_fmaf(kf, 0x1.344000p-2f, entry[0]);
	low = __builtin_fmaf(kf, 0x1.3509f8p-18f, entry[1]);
	scaled = r * 0x1.bcb7b2p-2f;
	low += __builtin_fmaf(r, 0x1.bcb7b2p-2f, -scaled) + __builtin_fmaf(r, -0x1.5b235ep-27f, __libc_fast_log1pf_tail(r) * 0x1.bcb7b2p-2f);

	return high + (scaled + low);
}
