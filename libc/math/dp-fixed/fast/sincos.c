/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sincos.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _GNU_SOURCE

#include <math.h>

#include "internal/math.h"
#include "dp-fixed-fast.h"

/*
 * The fast GNU sincos of the soft-double cores (PLAN D128, D197; docs/research/libm-fast-double.md, section 15): the
 * entry tests of sin and cos, one kernel call for both results, each rounded as the fast sin and cos round it, so each
 * equals them bit for bit; the double-double path where the kernel declines
 */
void sincos(double x, double *sinp, double *cosp)
{
	uint64_t a = __libc_trig_fixed_abs_bits(x);
	struct __libc_trig_fixed sine;
	struct __libc_trig_fixed cosine;

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

	if (__libc_trig_fixed(x, __LIBC_TRIG_FIXED_SIN | __LIBC_TRIG_FIXED_COS, &sine, &cosine)) {
		*sinp = __libc_dp_fixed_trig_round(&sine);
		*cosp = __libc_dp_fixed_trig_round(&cosine);
		return;
	}

	__libc_trig_dd(x, __LIBC_TRIG_FIXED_SIN | __LIBC_TRIG_FIXED_COS, sinp, cosp);
}
