/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincos.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

#include "internal/math.h"
#include "trig-fixed.h"

/*
 * GNU sincos in the speed flavour (P8.02, P8.03): the entry tests of sin and
 * cos, one kernel call for both results, and the double-double path only for
 * a result the kernel cannot round, so each stays bit-identical to this
 * flavour's sin and cos
 */
void sincos(double x, double *sinp, double *cosp)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed sine;
	struct __libc_trig_fixed cosine;
	unsigned int missing = __LIBC_TRIG_FIXED_SIN | __LIBC_TRIG_FIXED_COS;
	double s;
	double c;

	/* sin.c's and cos.c's entry tests on the bits */
	if (a > __LIBC_TRIG_FIXED_INF) {
		*sinp = x + x;
		*cosp = x + x;
		return;
	}
	if (a == __LIBC_TRIG_FIXED_INF) {
		*sinp = __libc_math_invalid();
		*cosp = __libc_math_invalid();
		return;
	}
	if (a < __LIBC_TRIG_FIXED_TINY) {
		volatile double tiny = 0x1p-80;

		*sinp = __libc_math_tiny(x, -x);
		*cosp = a == 0 ? 1.0 : 1.0 - tiny;
		return;
	}

	if (__libc_trig_fixed(x, missing, &sine, &cosine)) {
		if (__libc_trig_fixed_round(&sine, __LIBC_TRIG_FIXED_ERROR, sinp))
			missing &= ~__LIBC_TRIG_FIXED_SIN;
		if (__libc_trig_fixed_round(&cosine, __LIBC_TRIG_FIXED_ERROR, cosp))
			missing &= ~__LIBC_TRIG_FIXED_COS;
	}
	if (missing == 0)
		return;

	__libc_trig_dd(x, missing, &s, &c);
	if (missing & __LIBC_TRIG_FIXED_SIN)
		*sinp = s;
	if (missing & __LIBC_TRIG_FIXED_COS)
		*cosp = c;
}
