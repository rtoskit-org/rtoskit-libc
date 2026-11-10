/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast log2f of the single-precision FPU cores (PLAN D162, D168): x = 2^k z reduced against a table entry c_i
 * (__libc_fast_logf_reduce), k + log2 c_i + log1p(r) / ln2: k + log2 c_i's high part is exact; r / ln2 carries its rounding error by a fused multiply-add; so the last addition is the only large rounding. Below 1 ulp to nearest
 * (the note, section 8); float only, round to nearest only as the layer (D57).
 */
float log2f(float x)
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
	/* log2(+-0) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	r = __libc_fast_logf_reduce(x, &k, &i);
	entry = __libc_fast_logf_log2[i];
	kf = (float)k;
	high = kf + entry[0];
	low = entry[1];
	scaled = r * 0x1.715476p+0f;
	low += __builtin_fmaf(r, 0x1.715476p+0f, -scaled) + __builtin_fmaf(r, 0x1.4ae0c0p-26f, __libc_fast_log1pf_tail(r) * 0x1.715476p+0f);

	return high + (scaled + low);
}
