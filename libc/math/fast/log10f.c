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
#include <stdint.h>

#include "internal/math-fast.h"

/* The special values and 1 (C23 F.10.3.8; __libc_fast_logf_special) */
[[gnu::noinline]] static float log10f_special(float x)
{
	return __libc_fast_logf_special(x);
}

/*
 * The fast log10f of the double-FPU cores (PLAN D162, D169, P11.06b; docs/research/libm-fast-float-exp-log.md
 * section 18): x = 2^k z, log10f x = k log10 2 + (log c + log1p r) / ln10 in double with __libc_fast_logf_step (a 16-step table) and
 * __libc_fast_logf_log1p (degree 5, 2^-31.96 relative), no division, rounded once to float: below 0.51 ulp to nearest,
 * no hard-case correction. One integer test sends every argument but the positive normal floats other than 1 off the
 * common path; a positive subnormal comes back to it scaled by 2^23, its bits less 23 2^23 so that
 * __libc_fast_logf_step finds k - 23 (P11.06b).
 */
float log10f(float x)
{
	uint32_t ix = __libc_float_bits(x);
	double r;
	double l;
	int k;

	if (__builtin_expect(__libc_fast_logf_unusual(ix), 0)) {
		if (ix - 1u >= 0x007fffffu)
			return log10f_special(x);
		ix = __libc_float_bits(x * 0x1p23f) - (23u << 23);
	}
	l = __libc_fast_logf_step(ix, &r, &k);
	l += __libc_fast_logf_log1p(r);
	return (float)((double)k * 0x1.34413509f79ffp-2 + l * 0x1.bcb7b1526e50ep-2);
}
