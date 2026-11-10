/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2p1f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast log2p1f of the single-precision FPU cores (PLAN D162, D171): 1 + x reduced against a table entry c_i with
 * the rounding of 1 + x carried (__libc_fast_log1pf_reduce; r = x itself within 2^-5 of 0), then as log2f: k + log2 c_i's high part exact, r / ln2 with its rounding error by a fused multiply-add.
 * Below 1 ulp to nearest (the note, section 12); float only, round to nearest only as the layer (D57).
 */
float log2p1f(float x)
{
	const float *entry;
	float high;
	float low;
	float kf;
	float r;
	float lo;
	float scaled;
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
	/*
	 * Far below 1, x / ln2 with x first scaled by 2^64, so that the constant's low part does not underflow, and one
	 * rounding back (the low terms' underflow cost 0.9999 ulp near FLT_MIN, measured); +-0 keeps its sign
	 */
	if (__builtin_fabsf(x) < 0x1p-90f) {
		float t = x * 0x1p64f;

		if (x == 0)
			return x;
		return __builtin_fmaf(t, 0x1.715476p+0f, t * 0x1.4ae0c0p-26f) * 0x1p-64f;
	}

	r = __libc_fast_log1pf_reduce(x, &k, &i, &lo);
	entry = __libc_fast_logf_log2[i];
	kf = (float)k;
	high = kf + entry[0];
	low = entry[1];
	scaled = r * 0x1.715476p+0f;
	low += __builtin_fmaf(r, 0x1.715476p+0f, -scaled) + __builtin_fmaf(r, 0x1.4ae0c0p-26f, (__libc_fast_log1pf_tail(r) + lo) * 0x1.715476p+0f);

	return high + (scaled + low);
}
