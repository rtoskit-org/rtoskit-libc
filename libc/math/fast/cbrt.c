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

/* cbrt(2^j), j = 0..2, rounded */
static const double cbrt_scale[3] = { 1.0, 0x1.428a2f98d728bp+0, 0x1.965fea53d6e3dp+0 };

/*
 * The fast cbrt of the double-FPU cores (PLAN D162, D193): x = 2^(3q + j) m with m in [1, 2), cbrt(m) by a degree-4
 * Chebyshev fit (2^-16.2) times cbrt(2^j), one Newton step (2^-31), and one with the residual t^3 - y exact (the
 * products' errors by fused multiply-adds, t^3 - y itself exact by Sterbenz; or, without them, t cut to 26 bits and
 * Halley's step): about 2^-62 before the one rounding, t - d, with the sign on both. The exponent goes on after it (exactly: the result lies in [2^-358,
 * 2^342]).
 * C23 7.12.7.1, F.10.4.1: +-0, +-inf and NaN return themselves.
 */
double cbrt(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	int32_t e = (int32_t)((bits >> 52) & 0x7ff);
	struct __libc_dd square;
	struct __libc_dd cube;
	int32_t shift = 0;
	int32_t q;
	int32_t j;
	double m;
	double y;
	double t;
	double d;

	if (e == 0x7ff || (bits << 1) == 0)
		return x + x;
	if (e == 0) {
		bits = __libc_double_bits(x * 0x1p54);
		e = (int32_t)((bits >> 52) & 0x7ff);
		shift = -18;
	}

	/* q = floor((e - 1023)/3) by a multiply: e + 1200 - 1023 is positive and below 2^16 */
	q = (int32_t)(((uint32_t)(e + 177) * 43691u) >> 17) - 400;
	j = e - 1023 - 3 * q;
	m = __libc_double_from((bits & 0x000fffffffffffffull) | 0x3ff0000000000000ull);
	y = __libc_double_from((bits & 0x000fffffffffffffull) | ((uint64_t)(1023 + j) << 52));
	t = (0x1.04bc2bde493b9p-1 + m * (0x1.6c697f83452edp-1 + m * (-0x1.2d024eeecb9cbp-2 + m * (0x1.544aaec5e4624p-4 + m * -0x1.4b077fcd0882bp-7)))) * cbrt_scale[j];
	t = t - (t * t * t - y) / (3.0 * t * t);

	if (__libc_fpu_fma) {
		/* Newton's step with t^3 - y exact: two products' errors by fused multiply-adds */
		square = __libc_dd_product(t, t);
		cube = __libc_dd_product(square.hi, t);
		d = ((cube.hi - y) + (cube.lo + square.lo * t)) / (3.0 * square.hi);
	} else {
		/*
		 * Without one, Dekker's products cost 17 operations each (soft ones on the cores without a double FPU): t cut to
		 * 26 bits (2^-26) makes t^2 exact, its split a + b (27 bits and the rest) makes a t and b t exact, y - a t is
		 * exact (Sterbenz), and Halley's step, cubic, leaves 2^-78
		 */
		double a;
		double b;

		t = __libc_double_from(__libc_double_bits(t) & ~((1ull << 27) - 1));
		square.hi = t * t;
		a = __libc_double_from(__libc_double_bits(square.hi) & ~((1ull << 26) - 1));
		b = square.hi - a;
		d = -(t * ((y - a * t) - b * t)) / (2.0 * square.hi * t + y);
	}

	/* The sign on t and d before the one rounding, so that a directed rounding rounds the signed result */
	if (sign != 0) {
		t = -t;
		d = -d;
	}

	return __libc_double_from(__libc_double_bits(t - d) + ((uint64_t)(int64_t)(q + shift) << 52));
}
