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
 * The fast log2p1f of the double-FPU cores (PLAN D162, D171): within 1/4 of 0, log(1 + x) from x itself
 * (__libc_fast_log1p_double, 2^-39.5 relative however small x is); beyond, 1 + x in double, exact there, as 2^e m
 * (__libc_fast_log_split) and e + log m / ln2 with __libc_fast_log_double. Rounded once to float, below 0.51 ulp to
 * nearest (the note, section 12). Special values as C23 F.10.3.11.
 */
float log2p1f(float x)
{
	double f;
	double l;
	int e;

	if (isnan(x))
		return x + x;
	if (x < -1.0f)
		return __libc_math_invalidf();
	/* log(1 + -1) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == -1.0f)
		return -1.0f / fabsf(x + 1.0f);
	if (isinf(x))
		return x;
	/* Within 1/4 of 0, x itself is m - 1 (and e = 0 adds exactly nothing): 1 + x need not be exact */
	if (__builtin_fabsf(x) < 0.25f) {
		f = x;
		e = 0;
	} else {
		f = __libc_fast_log_split(1.0 + (double)x, &e) - 1.0;
	}

	l = __libc_fast_log1p_double(f) * 0x1.71547652b82fep+0;
	/* e = 0 adds nothing, and its +0 would turn log(1 + -0) = -0 into +0 */
	if (e != 0)
		l += (double)e;

	return (float)l;
}
