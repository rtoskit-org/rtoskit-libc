/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_trig_slow.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The slow path of the fast double sin, cos, sincos and tan (PLAN D196): the strict reduction (exact for every finite x,
 * D48) and the double-double cores in one round-to-nearest section, then the result as the strict file computes it:
 * sin(q pi/2 + r) for kind 0, cos for kind 1 (one more quadrant), tan for kind 2
 */
double __libc_fast_trig_slow(double x, int kind)
{
	struct __libc_dd r;
	struct __libc_dd sine;
	struct __libc_dd cosine;
	struct __libc_dd value;
	int quadrant;
	int mode;

	mode = __libc_math_nearest();
	quadrant = __libc_rem_pio2(x, &r);
	sine = __libc_sin_core(r);
	cosine = __libc_cos_core(r);
	if (kind == 2) {
		value = (quadrant & 1) == 0 ? __libc_dd_divide(sine, cosine) : __libc_dd_negate(__libc_dd_divide(cosine, sine));
	} else {
		quadrant += kind;
		value = (quadrant & 1) == 0 ? sine : cosine;
		if ((quadrant & 2) != 0)
			value = __libc_dd_negate(value);
	}
	__libc_math_restore(mode);

	return value.hi + value.lo;
}
