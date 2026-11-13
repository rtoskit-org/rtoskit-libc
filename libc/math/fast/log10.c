/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * log10.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast log10 of the double-FPU cores (PLAN D162, D190): log's hi + lo (its last sum not taken), times 1/ln 10 as a
 * pair with the product's error exact (fma or Dekker's), so that only the last addition rounds. Powers of 10 come out
 * exact (the error is far below half an ulp of the integer). C23 7.12.6.12, F.10.3.8.
 */
double log10(double x)
{
	const struct __libc_fast_log_entry *entry;
	uint64_t ix;
	struct __libc_dd product;
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

	entry = __libc_fast_log_reduce(__libc_fast_log_table, ix, &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	kd = (double)k;
	/* z = 1 exactly: k log10(2) rounded once (its head of 42 bits makes k times it exact), +0 for x = 1 */
	if (r == 0)
		return kd * 0x1.34413509f7800p-2 + kd * 0x1.fef311f12b358p-46;
	w = kd * 0x1.62e42fefa3800p-1 + entry->logc;
	hi = w + r;
	r2 = r * r;
	lo = (w - hi) + r + (kd * 0x1.ef35793c76730p-45 + entry->tail) + r2 * __libc_fast_log_poly(r, r2);
	product = __libc_dd_product(hi, 0x1.bcb7b1526e50ep-2);

	return product.hi + (product.lo + (lo * 0x1.bcb7b1526e50ep-2 + hi * 0x1.95355baaafad3p-57));
}
