/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast log of the double-FPU cores (PLAN D162, D190), in double with one rounding: x = 2^k z, r = z invc - 1 exact
 * (__libc_fast_log_r), w = k ln 2_hi + logc exact (ln 2_hi of 42 bits, logc a multiple of 2^-43), hi + lo = w + r by a
 * fast two-sum (|w| >= |r| wherever w is not 0), and lo + r^2 P(r) + hi with the tails k ln 2_lo and the entry's. Near
 * 1 (k = 0, invc = 1) w is 0 and the result r + r^2 P(r) relative however small r is. log(1) is +0 in every rounding
 * direction; the special values out of line (__libc_fast_log_special). C23 7.12.6.11, F.10.3.7.
 */
double log(double x)
{
	const struct __libc_fast_log_entry *entry;
	uint64_t ix;
	double kd;
	double r2;
	double hi;
	double lo;
	double w;
	double z;
	double r;
	int32_t k;

	if (__libc_fast_log_outside(x, &ix))
		return __libc_fast_log_special(x);
	/* +0 in every rounding direction (r = z invc - 1 would be -0 rounding downward) */
	if (ix == 0x3ff0000000000000ull)
		return 0.0;

	entry = __libc_fast_log_reduce(__libc_fast_log_table, ix, &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	kd = (double)k;
	w = kd * 0x1.62e42fefa3800p-1 + entry->logc;
	hi = w + r;
	lo = (w - hi) + r + (kd * 0x1.ef35793c76730p-45 + entry->tail);
	r2 = r * r;

	return lo + r2 * __libc_fast_log_poly(r, r2) + hi;
}
