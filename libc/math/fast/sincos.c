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
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast GNU sincos of the double-FPU cores (PLAN D128, D196; docs/research/libm-fast-double.md, section 14): sin's
 * and cos's entry tests, one reduction and one pair of polynomials for both kernels, so each result is the one the fast
 * sin or cos returns, bit for bit
 */
void sincos(double x, double *sinp, double *cosp)
{
	double a = __builtin_fabs(x);
	uint32_t n;
	double t;
	double t_lo;
	double z;
	double sp;
	double cp;
	double hi;
	double lo;

	if (isless(a, 0x1p-27)) {
		volatile double tiny = 0x1p-80;

		*sinp = __libc_math_tiny(x, -x);
		*cosp = x == 0 ? 1.0 : 1.0 - tiny;
		return;
	}
	if (!isless(a, 0x1p20)) {
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
		goto slow;
	}

	n = __libc_fast_rem_pio32(a, &t, &t_lo);
	z = t * t;
	sp = __libc_fast_sin_poly(z);
	cp = __libc_fast_cos_poly(z);

	/* Each result next to its own zero (sin's at n, cos's at n + 16) takes the slow path as sin or cos would */
	if (__libc_fast_trig_near_zero(n, t)) {
		*sinp = __libc_fast_trig_slow(x, 0);
	} else {
		hi = __libc_fast_sin_turn(n, t, t_lo, sp, cp, &lo);
		*sinp = __libc_fast_signed_sum(hi, lo, ((n & 32) != 0) != (signbit(x) != 0));
	}
	if (__libc_fast_trig_near_zero(n + 16, t)) {
		*cosp = __libc_fast_trig_slow(x, 1);
	} else {
		hi = __libc_fast_sin_turn(n + 16, t, t_lo, sp, cp, &lo);
		*cosp = __libc_fast_signed_sum(hi, lo, ((n + 16) & 32) != 0);
	}
	return;

slow:
	*sinp = __libc_fast_trig_slow(x, 0);
	*cosp = __libc_fast_trig_slow(x, 1);
}
