/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atanf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast atanf of the single-precision FPU cores (PLAN D162, D220, D235): atan |x| = c + s atan t with one reduction to
 * |t| <= 1/2: t = |x| up to 1/2; t = (|x| - 1)/(|x| + 1) below 2, c = pi/4 (__libc_fast_atanf_shift); t = 1/|x| from 2
 * rounded once, c = pi/2 and s = -1; then the polynomial (__libc_fast_atanf_poly) and c + s atan t
 * by Fast2Sum, the sign of x. Within D220's 0.9-ulp target to nearest over every float (the float trigonometric note,
 * section 27); round to nearest only as the layer (D57). C23 F.10.1.3: +-infinity gives +-pi/2, +-0 itself.
 */
float atanf(float x)
{
	float a = __builtin_fabsf(x);
	float t = a;
	float t_lo = 0.0f;
	float c = 0.0f;
	float c_lo = 0.0f;
	float hi;
	float lo;

	if (isnan(x))
		return x + x;
	/* x^3/3 is below half an ulp of x: the result rounds to x (keeping -0) */
	if (a < 0x1p-12f)
		return x;
	if (a >= 2.0f) {
		/*
		 * pi/2 - atan(1/a), +-infinity giving +-pi/2: 1/a rounded once, its error below 2^-26 absolute (u <= 1/2) and so
		 * below 0.125 ulp of a result above atan 2; its remainder by a fused multiply-add cost 6 instructions
		 */
		t = -1.0f / a;
		c = 0x1.921fb6p+0f;
		c_lo = -0x1.777a5cp-25f;
	} else if (a > 0.5f) {
		t = __libc_fast_atanf_shift(a, 1.0f, &t_lo);
		c = 0x1.921fb6p-1f;
		c_lo = -0x1.777a5cp-26f;
	}
	hi = __libc_fast_atanf_poly(t, t_lo, &lo);
	/* c + atan t = c - (-atan t), exact when c = 0 and by Fast2Sum otherwise (|atan t| < pi/4) */
	hi = __libc_fast_pair_from_larger(c, c_lo, -hi, -lo, &lo);
	hi = hi + lo;

	return x < 0 ? -hi : hi;
}
