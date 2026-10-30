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
 * C23 7.12.7.1: x = y * 2^(3q) with y in [1, 8). In round to nearest, Newton
 * steps from a linear guess give cbrt(y) to 2^-30, rounded to 20 bits so t^2
 * is exact, and one Halley step t + t(s - t)/(2t + s) with s = y/t^2 leaves
 * about 0.6 ulp. The residual y - t^3, in double-double, then corrects t by
 * (y - t^3)/(3t^2) to about 2^-100 relative, as hi + lo. The sign goes on
 * before the one rounding of hi + lo in the caller's direction, so upward
 * and downward stay apart for a negative x (docs/research/libm-vectors.md).
 * cbrt(y) is never a halfway point, and its scaling is exact: the result
 * lies in [2^-358, 2^342].
 */
double cbrt(double x)
{
	uint64_t bits = __libc_double_bits(x);
	int exponent = __libc_double_exponent(bits);
	int scale = 0;
	int mode;
	int q;
	double y;
	double t;
	double s;
	double residual;
	struct __libc_dd cube;
	struct __libc_dd root;

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

	mode = __libc_math_nearest();
	t = 1.0 + (y - 1.0) / 7.0;
	for (int i = 0; i < 5; ++i)
		t -= (t * t * t - y) / (3.0 * t * t);
	t = __libc_double_from(__libc_double_bits(t) & ~((1ULL << 33) - 1));
	s = y / (t * t);
	t += t * (s - t) / (t + t + s);

	/* y - t^3: y - cube.hi is exact (the two agree to about 52 bits) */
	cube = __libc_dd_multiply(__libc_dd_product(t, t), (struct __libc_dd){ .hi = t, .lo = 0.0 });
	residual = ((y - cube.hi) - cube.lo) / (3.0 * t * t);
	root = __libc_dd_fast_sum(t, residual);
	if (signbit(x))
		root = __libc_dd_negate(root);
	__libc_math_restore(mode);

	return scalbn(root.hi + root.lo, q + scale);
}
