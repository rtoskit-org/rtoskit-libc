/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expm1.c
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The ends (P11.08d1, D246), out of line so that the common path keeps no stack frame: NaN, the tiny arguments (x nudged
 * up, underflow for a subnormal: below 2^-54 the x^2/2 term is below half an ulp of x), from 64 e^x itself (the 1 is
 * below 2^-92 of it; overflow and +inf by exp's own slow path), and below -64 -1 + 2^-100, at run time so that it is
 * inexact (the fast files drop -frounding-math). C23 7.12.6.6, F.10.3.4.
 */
[[gnu::noinline]] static double expm1_ends(double x)
{
	volatile double tiny = 0x1p-100;

	if (isnan(x))
		return x + x;
	if (__builtin_fabs(x) < 0x1p-54)
		return __libc_math_tiny(x, 1.0);
	if (x > 0)
		return __libc_fast_exp_special(x);

	return isinf(x) ? -1.0 : tiny - 1.0;
}

/*
 * (e^u - 1 - u)/u^2 for |u| <= ln 2/128 by its Taylor terms to u^4 (u^2 q within 2^-64.8): where the result is
 * small next to 2^(k/64), exp's degree 4 (2^-62) would cost 0.13 ulp
 */
static double expm1_q(double u)
{
	double u2 = u * u;

	return 0x1p-1 + u * 0x1.5555555555555p-3 + u2 * (0x1.555555554e947p-5 + u * 0x1.111111110e10bp-7 + u2 * 0x1.6c16c16c16c17p-10);
}

/*
 * (e^x - 1 - x - x^2/2)/x^3 for |x| < 0.35 by its minimax fit of degree 9, fitted here (P11.08d1, D246; the note,
 * section 40): x^3 times it within 2^-59.7 of expm1 x, relatively
 */
static double expm1_direct(double x)
{
	double p = __libc_fast_fma(x, 0x1.1f960bab0f0c1p-29, 0x1.af637478a4b67p-26);

	p = __libc_fast_fma(x, p, 0x1.27e4c5ae58d74p-22);
	p = __libc_fast_fma(x, p, 0x1.71ddf5d87c5cap-19);
	p = __libc_fast_fma(x, p, 0x1.a01a01b0536c3p-16);
	p = __libc_fast_fma(x, p, 0x1.a01a01b0c7dc3p-13);
	p = __libc_fast_fma(x, p, 0x1.6c16c16c147bdp-10);
	p = __libc_fast_fma(x, p, 0x1.111111110f4bdp-7);
	p = __libc_fast_fma(x, p, 0x1.5555555555559p-5);

	return __libc_fast_fma(x, p, 0x1.5555555555559p-3);
}

/*
 * The fast expm1 of the double-FPU cores (PLAN D162, D187, D246), in double with one rounding. One test sends 2^-54 <=
 * |x| < 64 down the common paths, the rest to expm1_ends:
 * - below 0.35 in magnitude, x + x^2/2 + x^3 P(x) with no reduction: x^2/2 (up to 15 % of the result) as an exact pair
 *   by a fused multiply-add and x + x^2/2 by a fast two-sum, the rest (below 2 %) rounded;
 * - from 0.35, exp's reduction u = x - k ln2/64 and 2^(k/64) - 1 + 2^(k/64) (tail + u + u^2 q): |2^(k/64) - 1| is
 *   at least 0.29 there, so the second term (below 2 % of the result) rounds plainly, with expm1's own q; 2^(k/64) - 1 exactly as a pair
 *   (one fast two-sum, the larger first by k's sign; with no low part from 1/2 to 2^53).
 */
double expm1(double x)
{
	const uint64_t *entry;
	struct __libc_dd sum;
	double minus_lo;
	double minus;
	double scale;
	double kd;
	double u;
	double z;
	int32_t k;

	if (__libc_fast_exp_outside(x, 0x3c9, 0x405))
		return expm1_ends(x);

	if (__builtin_fabs(x) < 0.35) {
		z = x * x;
		sum = __libc_dd_fast_sum(x, 0.5 * z);
		return sum.hi + __libc_fast_fma(x * z, expm1_direct(x), sum.lo + __libc_fast_product(x, x).lo * 0.5);
	}

	k = __libc_fast_exp_index(x * 0x1.71547652b82fep+6, &kd);
	u = (x - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39;
	entry = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, k >> 6));
	minus = scale - 1.0;
	minus_lo = k < 0 ? scale - (minus + 1.0) : (scale - minus) - 1.0;

	/* expm1's own q (exp's polynomial, 2^-57 absolutely, would cost 0.15 ulp where the result is near 0.47) */
	return minus + __libc_fast_fma(scale, u + u * u * expm1_q(u), __libc_fast_fma(scale, __libc_double_from(entry[1]), minus_lo));
}
