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
 * The fast logf of the double-FPU cores (PLAN D162, D168): x = 2^e m, e * ln 2 + log m in double with
 * __libc_fast_log_double (2^-39.5), rounded once to float: below 0.51 ulp to nearest, log(1) = +0 exactly (s = 0); no hard-case
 * correction. Special values as C23 F.10.3.7.
 */
float logf(float x)
{
	int e;
	double m;

	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalidf();
	/* log(+-0) is -infinity with divide-by-zero: a division at run time, not a folded constant */
	if (x == 0)
		return -1.0f / fabsf(x);
	if (isinf(x))
		return x;

	/* +0 in every rounding direction: rounding downward, m - 1 would be -0 (C23 F.10.3.7) */
	if (x == 1.0f)
		return 0.0f;
	m = __libc_fast_logf_split(x, &e);

	return (float)((double)e * 0x1.62e42fefa39efp-1 + __libc_fast_log_double(m));
}
