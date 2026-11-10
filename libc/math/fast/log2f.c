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
 * The fast log2f of the double-FPU cores (PLAN D162, D168): x = 2^e m, e + log m / ln 2 in double with
 * __libc_fast_log_double (2^-39.5), rounded once to float: below 0.51 ulp to nearest, exact at the powers of two (m = 1); no hard-case
 * correction. Special values as C23 F.10.3.10.
 */
float log2f(float x)
{
	int e;
	double m;

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log2(+-0) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	/* +0 in every rounding direction: rounding downward, m - 1 would be -0 (C23 F.10.3.7) */
	if (x == 1.0f)
		return 0.0f;
	m = __libc_fast_logf_split(x, &e);

	return (float)((double)e + __libc_fast_log_double(m) * 0x1.71547652b82fep+0);
}
