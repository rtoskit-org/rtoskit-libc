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
 * The fast log1pf of the single-precision FPU cores (PLAN D162, D220, D237): within 2^-5 of 0, x + x^2 q(x)
 * (__libc_fast_log1pf_tail); beyond, u = 1 + x by a two-sum, u reduced against a table entry c_i as logf
 * (__libc_fast_logf_split_z), and the rounding c of 1 + x carried as c/u, one division (log(u + c) = log u + c/u to
 * c^2/u^2, below 2^-47). The specials and the small path go by unsigned tests on the bits. Within D220's 0.9-ulp target to nearest over every
 * float (the float trigonometric note, section 28); round to nearest only as the layer (D57).
 */
float log1pf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	const float *entry;
	float high;
	float low;
	float kf;
	float u;
	float c;
	float z;
	float r;
	int k;
	int i;

	/*
	 * NaN, x <= -1 and +infinity by one unsigned test on the bits each side: the common path is a finite x >= 0 below
	 * 0x7f800000, or a negative one above -1 (0xbf800000)
	 */
	if (bits >= 0x7f800000u && bits - 0x80000000u >= 0x3f800000u) {
		if (isnan(x))
			return x + x;
		/* x < -1, -infinity included: invalid */
		if (x < -1.0f)
			return __libc_math_invalidf();
		if (isinf(x))
			return x;
		/* log(1 + -1) is -infinity with divide-by-zero: a division at run time, not a folded constant */
		return -1.0f / fabsf(x + 1.0f);
	}
	/* |x| < 2^-5 */
	if ((bits & 0x7fffffffu) < 0x3d000000u) {
		/* |x| < 2^-25: x^2/2 is below half an ulp of x (and may underflow) */
		if ((bits & 0x7fffffffu) < 0x33000000u)
			return x;
		return x + __libc_fast_log1pf_tail(x);
	}
	u = __libc_fast_two_sum(1.0f, x, &c);
	z = __libc_fast_logf_split_z(u, &k, &i);
	r = __builtin_fmaf(z, __libc_fast_logf_invc[i], -1.0f);
	entry = __libc_fast_logf_ln[i];
	kf = (float)k;
	high = __builtin_fmaf(kf, 0x1.62e400p-1f, entry[0]);
	low = __builtin_fmaf(kf, 0x1.7f7d1cp-20f, entry[1]);

	return high + (r + (low + (__libc_fast_log1pf_tail(r) + c / u)));
}
