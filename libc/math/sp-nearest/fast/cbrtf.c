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
 * The fast cbrtf of the single-precision FPU cores (PLAN D162, D171): |x| = 2^(3q + j) f, f in [1, 2) and
 * j in {0, 1, 2} (subnormals first scaled by 2^24); t from a degree 2 polynomial in f times cbrt(2^j) (2^-10.1),
 * one Newton step in float (about 2^-21), then one with the residual t^3 - m from exact products (the square's
 * error by a fused multiply-add), about 2^-40 before the last rounding. Below 1 ulp to nearest (the note, section
 * 13); float only, round to nearest only as the layer (D57).
 */
float cbrtf(float x)
{
	static const float roots[3] = { 1.0f, 0x1.428a30p+0f, 0x1.965feap+0f };
	uint32_t bits;
	float f;
	float m;
	float t;
	float square;
	float square_lo;
	float residual;
	int scale = 127;
	int e;
	int q;
	int j;

	if (isnan(x) || isinf(x) || x == 0)
		return x + x;

	bits = __libc_float_bits(__builtin_fabsf(x));
	if (bits < 0x00800000) {
		bits = __libc_float_bits(__builtin_fabsf(x) * 0x1p24f);
		scale -= 8;
	}
	e = (int)(bits >> 23) - 127;
	/* floor(e / 3) for e in [-126, 127], by a division of a positive number */
	q = (e + 129) / 3 - 43;
	j = e - 3 * q;
	bits &= 0x007fffff;
	f = __libc_float_from(bits | 0x3f800000);
	m = __libc_float_from(bits | (uint32_t)(127 + j) << 23);

	t = __builtin_fmaf(f, -0x1.de1966p-5f, 0x1.bbf74ep-2f);
	t = __builtin_fmaf(f, t, 0x1.405a14p-1f) * roots[j];
	t -= (t * t * t - m) / (3.0f * t * t);
	square = t * t;
	square_lo = __builtin_fmaf(t, t, -square);
	residual = __builtin_fmaf(square, t, -m) + square_lo * t;
	t -= residual / (3.0f * square);
	t *= __libc_float_from((uint32_t)(q + scale) << 23);

	return x < 0 ? -t : t;
}
