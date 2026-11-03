/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincos.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

#include "internal/math.h"

/*
 * GNU sincos (PLAN D128, D133, docs/research/sincos.md): sin's and cos's own
 * entry tests, then one reduction and both double-double cores in one
 * round-to-nearest section, so each result is the one sin or cos returns,
 * bit for bit, for half the reduction's cost.
 */
void sincos(double x, double *sinp, double *cosp)
{
	struct __libc_dd r;
	struct __libc_dd sine;
	struct __libc_dd cosine;
	int quadrant;
	int mode;

	if (isnan(x)) {
		*sinp = x + x;
		*cosp = x + x;
		return;
	}
	if (isinf(x)) {
		*sinp = __libc_math_invalid();
		*cosp = __libc_math_invalid();
		return;
	}
	if (fabs(x) < 0x1p-27) {
		volatile double tiny = 0x1p-80;

		*sinp = __libc_math_tiny(x, -x);
		*cosp = x == 0 ? 1.0 : 1.0 - tiny;
		return;
	}

	mode = __libc_math_nearest();
	quadrant = __libc_rem_pio2(x, &r);
	sine = __libc_sin_core(r);
	cosine = __libc_cos_core(r);
	__libc_math_restore(mode);

	/* sin(r + q pi/2) and cos(r + q pi/2) by the quadrant, as sin.c and cos.c select them */
	switch (quadrant) {
		case 0:
			*sinp = sine.hi + sine.lo;
			*cosp = cosine.hi + cosine.lo;
			break;
		case 1:
			*sinp = cosine.hi + cosine.lo;
			*cosp = __libc_dd_negate(sine).hi + __libc_dd_negate(sine).lo;
			break;
		case 2:
			*sinp = __libc_dd_negate(sine).hi + __libc_dd_negate(sine).lo;
			*cosp = __libc_dd_negate(cosine).hi + __libc_dd_negate(cosine).lo;
			break;
		default:
			*sinp = __libc_dd_negate(cosine).hi + __libc_dd_negate(cosine).lo;
			*cosp = sine.hi + sine.lo;
			break;
	}
}
