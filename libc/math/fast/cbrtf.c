/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cbrtf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast cbrtf of the double-FPU cores (PLAN D162, D171): |x| = 2^(3q + j) f in double (every float is a normal
 * double), f in [1, 2) and j in {0, 1, 2}; cbrt(2^j f) from a degree 3 polynomial in f times cbrt(2^j) (2^-13.2),
 * then one Halley step t (t^3 + 2m)/(2t^3 + m), cubic, to about 2^-39; rounded once to float, below 0.51 ulp to
 * nearest (the note, section 13). Special values as C23 F.10.4.1.
 */
float cbrtf(float x)
{
	static const double roots[3] = { 1.0, 0x1.428a2f98d728bp+0, 0x1.965fea53d6e3dp+0 };
	uint64_t bits;
	double f;
	double m;
	double t;
	double t3;
	int e;
	int q;
	int j;

	if (isnan(x) || isinf(x) || x == 0)
		return x + x;

	bits = __libc_double_bits(__builtin_fabs((double)x));
	e = (int)(bits >> 52) - 1023;
	/* floor(e / 3) for e in [-149, 127], by a division of a positive number */
	q = (e + 150) / 3 - 50;
	j = e - 3 * q;
	bits &= 0x000fffffffffffffull;
	f = __libc_double_from(bits | 0x3ff0000000000000ull);
	m = __libc_double_from(bits | (uint64_t)(1023 + j) << 52);

	/* Plain products: the start needs no fused multiply-add, which x86_64's baseline lacks (a call, 1,057 instructions) */
	t = f * 0x1.6ae260afe5091p-6 - 0x1.44f0d2e8403a6p-3;
	t = f * t + 0x1.296213a52f037p-1;
	t = (f * t + 0x1.1c90a1fb8969fp-1) * roots[j];
	t3 = t * t * t;
	t = t * (t3 + 2.0 * m) / (2.0 * t3 + m);
	t *= __libc_double_from((uint64_t)(q + 1023) << 52);

	return (float)(x < 0 ? -t : t);
}
