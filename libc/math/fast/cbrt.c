/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cbrt.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The ends (P11.08d2, D247), out of line: NaN, the infinities and the zeros return themselves (C23 7.12.7.1, F.10.4.1);
 * below 2^-968 (where the residual's products would underflow) and from 2^1022 (where the Halley step's 2 t^3 + |x|
 * could overflow) the argument scaled by 2^162 or 2^-162, exactly, and the root by 2^-54 or 2^54
 */
[[gnu::noinline]] static double cbrt_ends(double x)
{
	if (isnan(x) || isinf(x) || x == 0)
		return x + x;
	if (__builtin_fabs(x) < 1.0)
		return cbrt(x * 0x1p162) * 0x1p-54;

	return cbrt(x * 0x1p-162) * 0x1p54;
}

/*
 * The fast cbrt of the double-FPU cores (PLAN D162, D193, D247; docs/research/libm-fast-double.md, section 41), from
 * |x|'s bits with no exponent split: t from the high word over 3 plus a constant fitted here (within 2^-5), one Halley
 * step t (t^3 + 2a)/(2 t^3 + a) (2^-15.5), then r = (a - t^3)/t^3 with t^3 exact as a pair (the products' errors by
 * fused multiply-adds, or Dekker's products without them; a - t^3 exact by Sterbenz) and the root as t (1 + r)^(1/3) =
 * t + t r P(r) with P the series to r^3 (|r| below 2^-13.9, the next term 2^-60 of the result), rounded once with
 * the sign on t. One test sends 2^-968 <= |x| < 2^1022 down this path.
 */
double cbrt(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint32_t high = (uint32_t)(bits >> 32) & 0x7fffffff;
	double a = __builtin_fabs(x);
	struct __libc_dd square;
	struct __libc_dd cube;
	double t3;
	double t;
	double r;

	if (high - 0x03700000u >= 0x7fd00000u - 0x03700000u)
		return cbrt_ends(x);

	t = __libc_double_from((uint64_t)(high / 3 + 0x2a9f761cu) << 32);
	t3 = t * t * t;
	/* The quotient first: t (t^3 + 2a) would underflow near 2^-968 */
	t = t * ((t3 + 2.0 * a) / (2.0 * t3 + a));
	square = __libc_fast_product(t, t);
	cube = __libc_fast_product(square.hi, t);
	r = ((a - cube.hi) - (cube.lo + square.lo * t)) / cube.hi;
	if (bits >> 63)
		t = -t;

	return __libc_fast_fma(t, r * (0x1.5555555555555p-2 + r * (-0x1.c71c71c71c71cp-4 + r * 0x1.f9add3c0ca458p-5)), t);
}
