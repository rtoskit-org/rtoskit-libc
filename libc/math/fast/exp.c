/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp of the double-FPU cores (PLAN D162, D187), in double with one rounding: k = round(64 x/ln 2), u = x - k ln 2/64
 * (ln 2/64 in two parts, the first of 33 bits so that k times it is exact and the subtraction too), e^u - 1 by a polynomial
 * and 2^(k/64) from the table with its tail (__libc_fast_exp_scale). Overflow, underflow and the special values as C23 7.12.6.1, F.10.3.1.
 */
double exp(double x)
{
	const uint64_t *entry;
	double scale;
	double kd;
	int32_t k;
	double u;

	/* 2^-54 <= |x| < 512: the result is normal and needs no test of the ends */
	if (__libc_fast_exp_outside(x, 0x3c9, 0x408))
		return __libc_fast_exp_special(x);

	k = __libc_fast_exp_index(x * 0x1.71547652b82fep+6, &kd);
	u = (x - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39;

	entry = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, k >> 6));
	return scale + scale * __libc_fast_expm1_tail(u, __libc_double_from(entry[1]));
}
