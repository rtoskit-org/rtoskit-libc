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
 * (e^u - 1 - u)/u^2 for |u| <= ln 2/128 by its Chebyshev fit of degree 5 (u^2 q within 2^-71.5): where the terms below
 * cancel, exp's degree 4 (2^-62) would cost 0.13 ulp
 */
static double expm1_q(double u)
{
	double u2 = u * u;

	return 0x1p-1 + u * 0x1.5555555555555p-3 + u2 * (0x1.555555554e947p-5 + u * 0x1.111111110e10bp-7 + u2 * (0x1.6c170c642c955p-10 + u * 0x1.a01a4443ba778p-13));
}

/*
 * The fast expm1 of the double-FPU cores (PLAN D162, D187), in double with one rounding: exp's reduction, u = hi + lo with
 * hi = x - k c1 exact and lo = -k c2. For k = 0 the polynomial alone (relative however small x); else 2^(k/64) - 1 exactly
 * as a pair, plus 2^(k/64) hi as an exact product (one fma where the FPU has it, Dekker's splits otherwise), plus 2^(k/64)
 * (lo + u^2 q + tail (1 + hi)) rounded (below 2^-14 of the result), summed as pairs so that only the last addition rounds.
 * Where the two first terms cancel (|x| near ln 2/128) a rounded u or tail + p would cost up to 2.1 ulp and a dropped tail p
 * 0.17 (measured). From 2^60 the 1
 * is below 2^-8 of an ulp and exp's result stands. Overflow and the special values as C23 7.12.6.6, F.10.3.4 and the strict file;
 * below -40, -1 + 2^-100 (e^x is below 2^-57).
 */
double expm1(double x)
{
	struct __libc_dd minus;
	struct __libc_dd product;
	struct __libc_dd sum;
	double scale;
	double small;
	double kd;
	double hi;
	double lo;
	double u;
	double q;
	int32_t k;
	int32_t j;
	int32_t n;

	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? x : -1.0;
	if (x > 0x1.62e42fefa39efp+9)
		return __libc_math_overflow(1.0);
	if (x < -40.0) {
		/* At run time, so that it is inexact and rounds in every direction (the fast files drop -frounding-math) */
		volatile double tiny = 0x1p-100;

		return tiny - 1.0;
	}
	/* Below 2^-54 the x^2/2 term is below half an ulp of x: x nudged up, underflow for a subnormal (C23 7.12.6.6, F.10.3.4) */
	if (__builtin_fabs(x) < 0x1p-54)
		return __libc_math_tiny(x, 1.0);

	k = __libc_fast_exp_index(x * 0x1.71547652b82fep+6, &kd);
	hi = x - kd * 0x1.62e42fee00000p-7;
	lo = -kd * 0x1.a39ef35793c76p-39;
	u = hi + lo;
	q = expm1_q(u);
	if (k == 0)
		return x + x * x * q;

	j = k & 63;
	n = (k - j) / 64;
	if (n > 60)
		return __libc_fast_exp_scale(k, u + u * u * q);

	scale = __libc_double_from(__libc_fast_exp_scale_bits(&__libc_fast_exp_pairs[2 * j], n));
	minus = __libc_dd_sum(scale, -1.0);
	product = __libc_dd_product(scale, hi);
	small = scale * (lo + u * u * q + __libc_double_from(__libc_fast_exp_pairs[2 * j + 1]) * (1.0 + hi));
	sum = __libc_dd_sum(minus.hi, product.hi);

	return sum.hi + (sum.lo + (product.lo + minus.lo + small));
}
