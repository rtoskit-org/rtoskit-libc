/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cbrt.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * C23 7.12.7.1: x = y * 2^(3q) with y in [1, 8). Newton steps from a linear
 * guess give cbrt(y) to about 2^-40; t keeps its top 26 bits, so t^2 is
 * exact, and t^2 = a + b with a holding 27 bits makes t^3 = a t + b t
 * exactly. The residual r = y - t^3 is then exact to about 2^-80 (y - a t
 * by Sterbenz), and one Halley step t + t r / (2t^3 + y) leaves about 2^-78
 * relative error: below 0.5 + 2^-25 ulp. None of it needs round to
 * nearest. The sign goes on before the one rounding of t + d, so the
 * caller's direction applies to the signed result (docs/research/libm-vectors.md);
 * the scaling is exact, as the result lies in [2^-358, 2^342].
 */
double cbrt(double x)
{
	uint64_t bits = __libc_double_bits(x);
	int exponent = __libc_double_exponent(bits);
	int scale = 0;
	int q;
	double y;
	double t;
	double square;
	double a;
	double b;
	double residual;
	double d;

	if (exponent == 0x7ff || x == 0)
		return x + x;
	if (exponent == 0) {
		bits = __libc_double_bits(x * 0x1p54);
		exponent = __libc_double_exponent(bits);
		scale = -18;
	}

	exponent -= __LIBC_DOUBLE_BIAS;
	q = exponent >= 0 ? exponent / 3 : -((-exponent + 2) / 3);
	y = __libc_double_from((bits & __LIBC_DOUBLE_FRACTION) | ((uint64_t)(__LIBC_DOUBLE_BIAS + exponent - 3 * q) << 52));

	t = 1.0 + (y - 1.0) / 7.0;
	for (int i = 0; i < 5; ++i)
		t -= (t * t * t - y) / (3.0 * t * t);
	t = __libc_double_from(__libc_double_bits(t) & ~((1ULL << 27) - 1));
	square = t * t;
	a = __libc_double_from(__libc_double_bits(square) & ~((1ULL << 26) - 1));
	b = square - a;
	residual = (y - a * t) - b * t;
	d = t * residual / (2.0 * square * t + y);
	if (signbit(x)) {
		t = -t;
		d = -d;
	}

	return scalbn(t + d, q + scale);
}
