/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * asin.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast asin of the double-FPU cores (PLAN D162, D198; docs/research/libm-fast-double.md, section 16): x + x z P(z)
 * up to 1/2, and pi/2 - 2 asin(sqrt((1 - |x|)/2)) above, with the square root's remainder; one rounding. C23 7.12.4.2,
 * F.10.1.2: |x| > 1 is a domain error; +-0 returns itself. Where the FPU has a fused multiply-add, asin_fused below.
 */

/*
 * The fused path (P11.08c, D244; the note, section 39), at the double-FPU cores' 0.9-ulp target (D245): the same forms
 * in plain double, the series as one rational (__libc_fast_asin_w) and the tail rounded once; 0.7059 ulp below 1/2 and 0.7435 above over
 * the section's samples
 */
/* The tiny arguments out of line, so that the fused path needs no stack frame: their helpers make calls */
[[gnu::noinline]] static double asin_tiny(double x)
{
	return __libc_math_tiny(x, x);
}

static double asin_fused(double x, double a)
{
	/* |x|'s high word: the tests below are integer comparisons, and a NaN is above 1 */
	uint32_t high = (uint32_t)(__libc_double_bits(a) >> 32);
	double t;
	double s;
	struct __libc_dd first;

	if (high < 0x3e400000)
		return asin_tiny(x);
	if (high >= 0x3ff00000) {
		if (a == 1.0)
			return __libc_fast_signed_sum(__LIBC_PI_2.hi, __LIBC_PI_2.lo, signbit(x) != 0);
		return isnan(x) ? x + x : __libc_math_invalid();
	}
	s = __libc_fast_asin_fused(a, a, high < 0x3fe00000, &t);
	/* a + t below 1/2 */
	if (high < 0x3fe00000)
		return __libc_fast_signed_sum(a, t, signbit(x) != 0);
	/* pi/2 - 2 s - 2 t by one fast two-sum: the result is at least 0.52, 2 t below 0.05 */
	first = __libc_dd_fast_sum(__LIBC_PI_2.hi, -2.0 * s);

	return __libc_fast_signed_sum(first.hi, (first.lo + __LIBC_PI_2.lo) - 2.0 * t, signbit(x) != 0);
}

double asin(double x)
{
	double a = __builtin_fabs(x);
	double hi;
	double lo;

	if (__libc_fpu_fma)
		return asin_fused(x, a);

	/* A NaN compares quietly (isless, islessequal): the relational operators would raise invalid */
	if (isless(a, 0x1p-27))
		return __libc_math_tiny(x, x);
	if (islessequal(a, 0.5)) {
		/* a + a w: z = a^2 as an exact pair, a w_hi exact, one fast two-sum (a w < a/20) */
		struct __libc_dd z = __libc_dd_product(a, a);
		double t_lo;
		double t = __libc_fast_asin_series(a, z.hi, z.lo, &t_lo);
		struct __libc_dd sum = __libc_dd_fast_sum(a, t);

		hi = sum.hi;
		lo = sum.lo + t_lo;
	} else if (isless(a, 1.0)) {
		/* pi/2 - 2 asin s: two fast two-sums (the result is at least 0.52, 2 s w below 0.05) */
		double rest;
		double t_lo;
		double t;
		double s = __libc_fast_asin_half(a, &rest, &t_lo, &t);
		struct __libc_dd first = __libc_dd_fast_sum(__LIBC_PI_2.hi, -2.0 * s);
		struct __libc_dd second = __libc_dd_fast_sum(first.hi, -2.0 * t);

		hi = second.hi;
		lo = (second.lo + first.lo + __LIBC_PI_2.lo) - 2.0 * (rest + t_lo);
	} else if (a == 1.0) {
		hi = __LIBC_PI_2.hi;
		lo = __LIBC_PI_2.lo;
	} else {
		return isnan(x) ? x + x : __libc_math_invalid();
	}

	return __libc_fast_signed_sum(hi, lo, signbit(x) != 0);
}
