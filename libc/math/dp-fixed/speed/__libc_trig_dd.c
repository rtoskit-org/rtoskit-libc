/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_trig_dd.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "trig-fixed.h"

/*
 * The double-double path of sin.c and cos.c for an argument past their
 * entry tests, for the results the fixed-point kernel declines or cannot
 * round: one reduction, the cores in round-to-nearest, each result negated
 * as a double-double before the final sum, as sin.c and cos.c do
 */
void __libc_trig_dd(double x, unsigned int want, double *sine, double *cosine)
{
	struct __libc_dd r;
	struct __libc_dd s = { 0 };
	struct __libc_dd c = { 0 };
	struct __libc_dd value;
	int quadrant;
	int mode;

	mode = __libc_math_nearest();
	quadrant = __libc_rem_pio2(x, &r);
	/* An odd quadrant swaps the cores: sin x needs cos r there, and cos x sin r */
	if (want & (quadrant & 1 ? __LIBC_TRIG_FIXED_COS : __LIBC_TRIG_FIXED_SIN))
		s = __libc_sin_core(r);
	if (want & (quadrant & 1 ? __LIBC_TRIG_FIXED_SIN : __LIBC_TRIG_FIXED_COS))
		c = __libc_cos_core(r);
	__libc_math_restore(mode);

	if (want & __LIBC_TRIG_FIXED_SIN) {
		value = quadrant & 1 ? c : s;
		if (quadrant >= 2)
			value = __libc_dd_negate(value);
		*sine = value.hi + value.lo;
	}
	if (want & __LIBC_TRIG_FIXED_COS) {
		value = quadrant & 1 ? s : c;
		if (quadrant == 1 || quadrant == 2)
			value = __libc_dd_negate(value);
		*cosine = value.hi + value.lo;
	}
}
