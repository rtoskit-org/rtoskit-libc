/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_tan_dd.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math.h"
#include "trig-fixed.h"

/* tan.c's double-double path, for the results the fixed-point quotient cannot round (P9.03) */
double __libc_tan_dd(double x)
{
	struct __libc_dd r;
	struct __libc_dd value;
	int quadrant;
	int mode;

	mode = __libc_math_nearest();
	quadrant = __libc_rem_pio2(x, &r);
	if ((quadrant & 1) == 0)
		value = __libc_dd_divide(__libc_sin_core(r), __libc_cos_core(r));
	else
		value = __libc_dd_negate(__libc_dd_divide(__libc_cos_core(r), __libc_sin_core(r)));
	__libc_math_restore(mode);
	return value.hi + value.lo;
}
