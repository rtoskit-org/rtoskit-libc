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
 * The fast cbrtf of the single-precision FPU cores (PLAN D162, D220, D237), on the bits with no exponent split: t0
 * from x's bits divided by 3 plus an offset (0x2a50e000, chosen here to minimise the span of t0^3/x over the
 * significands of three binades: within 9.5 % of the root), the sign kept; then d = t0^3/x - 1 and
 * t = t0 (1 + d)^(-1/3) by a degree-2 polynomial (2^-14.7), and one Newton step with the residual t^2 t - x by a fused
 * multiply-add, the square rounded (2^-25.6 relative at most) before the last rounding. Below 2^-100 x is scaled by
 * 2^72 first; zeros, infinities and NaNs go by one unsigned test. Within D220's 0.9-ulp target to nearest over every
 * float (the float trigonometric note, section 28); round to nearest only as the layer (D57).
 */
float cbrtf(float x)
{
	uint32_t bits = __libc_float_bits(__builtin_fabsf(x));
	float scale = 1.0f;
	float t;
	float d;
	float square;
	float residual;

	/* +-0, +-infinity and NaN: x + x */
	if (bits - 1 >= 0x7f7fffffu)
		return x + x;
	/* Below 2^-100 the square's error times t underflows: scaled by 2^72 (the result by 2^-24), subnormals included */
	if (bits < 0x0d800000) {
		x *= 0x1p72f;
		bits = __libc_float_bits(__builtin_fabsf(x));
		scale = 0x1p-24f;
	}
	t = __libc_float_from((bits / 3 + 0x2a50e000u) | (__libc_float_bits(x) & 0x80000000u));
	/* t^2 (t/x) rather than t^3/x, which overflows near FLT_MAX */
	d = t * t * (t / x) - 1.0f;
	t *= __builtin_fmaf(d, __builtin_fmaf(d, 0x1.ca02e2p-3f, -0x1.568b0ap-2f), 0x1.ffffbep-1f);
	/* The square rounded: its error moves t by at most 2^-25.6 relative (0.17 ulp), within D220's target */
	square = t * t;
	residual = __builtin_fmaf(square, t, -x);
	t -= residual / (3.0f * square);

	return t * scale;
}
