/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sin.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast sin of the double-FPU cores (PLAN D162, D196; docs/research/libm-fast-double.md, section 14): |x| = n pi/32 + t
 * by Cody-Waite in double, sin or cos of j pi/32 + t from the table's exact pairs and two short polynomials, the leading
 * terms combined exactly so that only the last addition rounds. From 2^20, and next to a zero, the strict reduction and
 * cores. C23 7.12.4.6, F.10.1.6: an infinity is a domain error; +-0 returns itself.
 */
double sin(double x)
{
	double a = __builtin_fabs(x);
	uint32_t n;
	double t;
	double t_lo;
	double z;
	double hi;
	double lo;

	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (isless(a, 0x1p-27))
		return __libc_math_tiny(x, -x);
	if (!isless(a, 0x1p20)) {
		if (isnan(x))
			return x + x;
		if (isinf(x))
			return __libc_math_invalid();
		return __libc_fast_trig_slow(x, 0);
	}

	n = __libc_fast_rem_pio32(a, &t, &t_lo);
	if (__libc_fast_trig_near_zero(n, t))
		return __libc_fast_trig_slow(x, 0);

	z = t * t;
	hi = __libc_fast_sin_turn(n, t, t_lo, __libc_fast_sin_poly(z), __libc_fast_cos_poly(z), &lo);

	/* sin is odd, and bit 5 of n is the half turn */
	return __libc_fast_signed_sum(hi, lo, ((n & 32) != 0) != (signbit(x) != 0));
}
