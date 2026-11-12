/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp2.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp2 of the double-FPU cores (PLAN D162, D187), in double with one rounding: k = round(64 x), r = x - k/64 exactly,
 * 2^r - 1 by its own polynomial (no product r ln 2 first) and 2^(k/64) from the table with its tail. Overflow, underflow and the special values as C23 7.12.6.4, F.10.3.2.
 */
/*
 * The slow path: tiny arguments, the ends, the infinities and NaN, by the strict file's double-double core (rare, and the
 * core is shared with pow, so in a program that calls both it costs no code); out of line, so that the common path saves
 * nothing
 */
[[gnu::noinline]] static double exp2_special(double x)
{
	struct __libc_dd value;
	struct __libc_dd z;
	int exponent;

	if (isnan(x))
		return x + x;
	if (__builtin_fabs(x) < 0x1p-54)
		return 1.0 + x;
	if (x >= 1024.0)
		return isinf(x) ? x : __libc_math_overflow(1.0);
	if (x < -1075.0)
		return isinf(x) ? 0.0 : __libc_math_underflow(1.0);
	/* Integers exactly, with no exception (the strict file's rule: 2^-1074 is not an underflow) */
	if (x == trunc(x))
		return scalbn(1.0, (int)x);
	z = __libc_dd_product(x, __LIBC_LN2.hi);
	z = __libc_dd_fast_sum(z.hi, z.lo + x * __LIBC_LN2.lo);
	value = __libc_exp_core(z.hi, z.lo, &exponent);
	return __libc_exp_scale(value, exponent);
}

double exp2(double x)
{
	const uint64_t *entry;
	double scale;
	double kd;
	int32_t k;

	/* 2^-54 <= |x| < 512: the result is normal and needs no test of the ends */
	if (__libc_fast_exp_outside(x, 0x3c9, 0x408))
		return exp2_special(x);

	k = __libc_fast_exp_index(x * 64.0, &kd);

	entry = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, k >> 6));
	return scale + scale * __libc_fast_exp2m1_tail(x - kd * 0x1p-6, __libc_double_from(entry[1]));
}
