/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * exp10.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast exp10 of the double-FPU cores (PLAN D162, D187), in double with one rounding: k = round(64 x log2 10), r = x -
 * k log10(2)/64 (in two parts, the first of 31 bits), 10^r - 1 = e^(r ln 10) - 1 by exp's polynomial and 2^(k/64) from the
 * table with its tail. Overflow, underflow and the special values as C23 7.12.6.2, F.10.3.3.
 */
/*
 * The slow path: tiny arguments, the ends, the infinities and NaN, by the strict file's double-double core (rare, and the
 * core is shared with pow, so in a program that calls both it costs no code); out of line, so that the common path saves
 * nothing
 */
[[gnu::noinline]] static double exp10_special(double x)
{
	struct __libc_dd value;
	struct __libc_dd z;
	int exponent;

	if (isnan(x))
		return x + x;
	/* 10^x rounds as 1 + x ln 10 (1 + x alone was 1.15 ulp below 1, measured) */
	if (__builtin_fabs(x) < 0x1p-54)
		return 1.0 + x * 0x1.26bb1bbb55516p+1;
	if (x > 0x1.34413509f79fep+8)
		return isinf(x) ? x : __libc_math_overflow(1.0);
	if (x < -0x1.439b746e36b52p+8)
		return isinf(x) ? 0.0 : __libc_math_underflow(1.0);
	z = __libc_dd_product(x, __LIBC_LN10.hi);
	z = __libc_dd_fast_sum(z.hi, z.lo + x * __LIBC_LN10.lo);
	value = __libc_exp_core(z.hi, z.lo, &exponent);
	return __libc_exp_scale(value, exponent);
}

double exp10(double x)
{
	const uint64_t *entry;
	double scale;
	double kd;
	int32_t k;
	double r;

	/* 2^-54 <= |x| < 256: the result is normal and needs no test of the ends */
	if (__libc_fast_exp_outside(x, 0x3c9, 0x407))
		return exp10_special(x);

	k = __libc_fast_exp_index(x * 0x1.a934f0979a371p+7, &kd);
	r = (x - kd * 0x1.3441350800000p-8) - kd * 0x1.f79fef311f12bp-40;

	entry = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, k >> 6));
	return scale + scale * __libc_fast_expm1_tail(r * 0x1.26bb1bbb55516p+1, __libc_double_from(entry[1]));
}
