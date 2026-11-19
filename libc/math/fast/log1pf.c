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
 * The ends (P11.08d1, D246), out of line so that the common path keeps no stack frame: NaN, below -1 a domain error,
 * log(1 + -1) = -infinity with divide-by-zero (a division at run time, not a folded constant), +infinity, and the tiny
 * arguments as the other build types (x, with underflow for a subnormal one). C23 F.10.3.9.
 */
[[gnu::noinline]] static float log1pf_ends(float x)
{
	if (isnan(x))
		return x + x;
	if (x < -1.0f)
		return __libc_math_invalidf();
	if (x == -1.0f)
		return -1.0f / fabsf(x + 1.0f);
	if (isinf(x))
		return x;

	return __libc_math_tinyf(x, -1.0);
}

/*
 * The fast log1pf of the double-FPU cores (PLAN D162, D171, D246): within 1/4 of 0, log(1 + x) from x itself
 * (__libc_fast_log1p_double, 2^-39.5 relative however small x is); beyond, 1 + x in double, exact there, as 2^e m
 * (__libc_fast_log_split) and e ln2 + log m with __libc_fast_log_double. Rounded once to float, below 0.51 ulp to
 * nearest (the note, section 12). One integer test sends 2^-30 <= |x| and -1 < x < +infinity down the common path.
 */
float log1pf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t a = bits & 0x7fffffff;
	uint32_t top = (bits >> 31) != 0 ? 0x3f800000u : 0x7f800000u;
	double f;
	double l;
	int e;

	if (a - 0x30800000u >= top - 0x30800000u)
		return log1pf_ends(x);
	/* Within 1/4 of 0, x itself is m - 1 (and e = 0 adds exactly nothing): 1 + x need not be exact */
	if (a < 0x3e800000u) {
		f = x;
		e = 0;
	} else {
		f = __libc_fast_log_split(1.0 + (double)x, &e) - 1.0;
	}

	l = __libc_fast_log1p_double(f);
	/* e = 0 adds nothing, and its +0 would turn log(1 + -0) = -0 into +0 */
	if (e != 0)
		l += (double)e * 0x1.62e42fefa39efp-1;

	return (float)l;
}
