/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log2.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * (log2(1 + r) - r/ln 2)/r^2 for |r| <= 2^-7.47 by its Chebyshev fit of degree 5 in double (r^2 times it within 2^-59.7
 * of log2(1 + r), relatively)
 */
static inline double log2_poly(double r, double r2)
{
	return -0x1.71547652b8300p-1 + r * 0x1.ec709dc3a0400p-2 + r2 * (-0x1.71547650f4a2ep-2 + r * 0x1.2776c50d685efp-2 + r2 * (-0x1.ec7520d6b23b6p-3 + r * 0x1.a61b656377ec0p-3));
}

/*
 * The fast log2 of the double-FPU cores (PLAN D162, D190), log's method with the base-2 table: r/ln 2 as t + te, exact
 * to about 2^-74 of r (a fused multiply-add for the product's error; or r's top 21 bits times 1/ln 2 to 32 bits, exact,
 * plus the rest of r times it and r times the rest of 1/ln 2), w = k + log2c exact (a multiple of 2^-42), and the rest as
 * log's. A power of 2 gives r = 0
 * and k at once. C23 7.12.6.13, F.10.3.9.
 */
double log2(double x)
{
	const struct __libc_fast_log_entry *entry;
	uint64_t ix;
	double r2;
	double hi;
	double lo;
	double te;
	double t;
	double w;
	double z;
	double r;
	int32_t k;

	if (__libc_fast_log_outside(x, &ix))
		return __libc_fast_log_special(x);

	entry = __libc_fast_log_reduce(__libc_fast_log2_table, ix, &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	/* z = 1 exactly: a power of 2, k exactly (+0 for x = 1 in every direction) */
	if (r == 0)
		return (double)k;
	if (__libc_fpu_fma) {
		t = r * 0x1.71547652b82fep+0;
		te = __builtin_fma(r, 0x1.71547652b82fep+0, -t) + r * 0x1.777d0ffda0d24p-56;
	} else {
		double high = __libc_double_from(__libc_double_bits(r) & 0xffffffff00000000ull);

		t = high * 0x1.71547652p+0;
		te = (r - high) * 0x1.71547652p+0 + r * 0x1.705fc2eefa200p-33;
	}
	w = (double)k + entry->logc;
	hi = w + t;
	lo = (w - hi) + t + (te + entry->tail);
	r2 = r * r;

	return lo + r2 * log2_poly(r, r2) + hi;
}
