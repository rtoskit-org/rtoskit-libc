/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan2f.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atan2f of the single-precision FPU cores (PLAN D162, D220, D235): atan(|y|/|x|) or pi/2 - atan(|x|/|y|) with one reduction to |t| <= 1/2 and one polynomial (__libc_fast_atanf_poly), the result k pi/4 + s atan t by one Fast2Sum, the sign of y. Within D220's 0.9-ulp target to nearest (the float trigonometric note, section 27); float only, round to nearest only as the layer (D57). The zeros, infinities and NaNs as C23 F.10.1.4 (the other build types' cases).
 */
float atan2f(float y, float x)
{
	float ax;
	float ay;
	/* k pi/4 as float pairs, k = 0 .. 4 */
	static const float quarter_pi[5][2] = { { 0.0f, 0.0f }, { 0x1.921fb6p-1f, -0x1.777a5cp-26f }, { 0x1.921fb6p+0f, -0x1.777a5cp-25f }, { 0x1.2d97c8p+1f, -0x1.99bc5cp-28f }, { 0x1.921fb6p+1f, -0x1.777a5cp-24f } };
	float n;
	float d;
	float t;
	float t_lo;
	float hi;
	float lo;
	bool swap;
	int k;

	ax = __builtin_fabsf(x);
	ay = __builtin_fabsf(y);
	/* Both in [2^-100, 2^60) on the common path: one unsigned test each, the zeros, infinities and NaNs and the scaling below */
	if (__libc_float_bits(ax) - 0x0d800000u >= 0x50000000u || __libc_float_bits(ay) - 0x0d800000u >= 0x50000000u) {
		if (isnan(x) || isnan(y))
			return x + y;
		if (y == 0)
			return signbit(x) ? copysignf(0x1.921fb6p1f, y) : y;
		if (x == 0)
			return copysignf(0x1.921fb6p0f, y);
		if (isinf(y))
			return copysignf(isinf(x) ? (x > 0 ? 0x1.921fb6p-1f : 0x1.2d97c8p1f) : 0x1.921fb6p0f, y);
		if (isinf(x))
			return x > 0 ? copysignf(0.0f, y) : copysignf(0x1.921fb6p1f, y);
		/*
		 * A small argument scaled by 2^64 with the other (both exact), so that the quotient's remainder does not fall below
		 * the subnormals (it measured 1.4 ulp); with the other from 2^60 the quotient is below 2^-160 and needs no remainder
		 */
		if ((ax < 0x1p-100f || ay < 0x1p-100f) && ax < 0x1p60f && ay < 0x1p60f) {
			ax *= 0x1p64f;
			ay *= 0x1p64f;
		}
	}
	/*
	 * atan(n/d) with n <= d: by t = n/d (its remainder by a fused multiply-add) up to 1/2, else pi/4 + atan t with
	 * t = (n - d)/(n + d) from the operands (__libc_fast_atanf_shift; n + d scaled by 1/4 near the overflow); then the
	 * result is k pi/4 + s atan t, k and s from the swap and x's sign, one Fast2Sum
	 */
	swap = ay > ax;
	n = swap ? ax : ay;
	d = swap ? ay : ax;
	k = 0;
	if (n > 0.5f * d) {
		if (d > 0x1p126f) {
			n *= 0.25f;
			d *= 0.25f;
		}
		t = __libc_fast_atanf_shift(n, d, &t_lo);
		k = 1;
	} else {
		t = n / d;
		/*
		 * atan t = t (t^3/3 below 2^-120 of t), and t is the quotient rounded once: below 2^-102 its remainder would round
		 * on the subnormal grid, up to half an ulp of t, and adding it back measured 0.67 ulp (complex_float, clogf)
		 */
		if (t < 0x1p-60f && x > 0 && !swap)
			return y < 0 ? -t : t;
		t_lo = __builtin_fmaf(-t, d, n) / d;
	}
	hi = __libc_fast_atanf_poly(t, t_lo, &lo);
	if (swap) {
		k = 2 - k;
		hi = -hi;
		lo = -lo;
	}
	if (x < 0) {
		k = 4 - k;
		hi = -hi;
		lo = -lo;
	}
	/* k pi/4 + (hi + lo) = k pi/4 - (-hi - lo), exact when k = 0 and by Fast2Sum otherwise (|atan t| < pi/4) */
	hi = __libc_fast_pair_from_larger(quarter_pi[k][0], quarter_pi[k][1], -hi, -lo, &lo);
	hi = hi + lo;

	return y < 0 ? -hi : hi;
}
