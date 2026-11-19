/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expm1f.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>

#include "internal/math-fast.h"

/*
 * The fast expm1f of the single-precision FPU cores (PLAN D162, D220, D237). Up to |x| = 1 no reduction:
 * x + x^2/2 + x^3 P(x) with P of degree 6 fitted on [-1, 1] (relative error 2^-26.9 with float coefficients), x^2 and x^3 as
 * exact pairs, x^3 P0 inside one fused multiply-add and x + x^2/2 by a two-sum, so that only the last addition rounds much. Beyond, x = k ln2 + u with
 * u_hi = x - k ln2f exact by a fused multiply-add and u_lo = k (ln2f - ln2), as expf, and e^u 2^k - 1 by
 * __libc_fast_expm1f_kernel. Within D220's 0.9-ulp target to nearest over every float (the float trigonometric note,
 * section 28); round to nearest only as the layer (D57).
 */
float expm1f(float x)
{
	float square;
	float square_lo;
	float cube;
	float cube_lo;
	float hi;
	float error;
	float p;
	float k;

	/* A quiet comparison: NaN goes on to the tests below */
	if (islessequal(__builtin_fabsf(x), 1.0f)) {
		/* x^2/2 is below half an ulp of x (and may underflow) */
		if (__builtin_fabsf(x) < 0x1p-25f)
			return x;
		square = x * x;
		p = __builtin_fmaf(x, 0x1.70cf92p-19f, 0x1.a995c0p-16f);
		p = __builtin_fmaf(x, p, 0x1.a04370p-13f);
		p = __builtin_fmaf(x, p, 0x1.6bfed4p-10f);
		p = __builtin_fmaf(x, p, 0x1.11109ep-7f);
		p = __builtin_fmaf(x, p, 0x1.55557ap-5f);
		square_lo = __builtin_fmaf(x, x, -square);
		cube = x * square;
		cube_lo = __builtin_fmaf(x, square, -cube) + x * square_lo;
		hi = __libc_fast_two_sum(x, 0.5f * square, &error);
		/* x^3 as a pair and x^3 P0 inside one fused multiply-add: rounded apart, the x^3/6 term cost 0.1 ulp near -1 */
		return hi + (error + __builtin_fmaf(cube, 0x1.555556p-3f, __builtin_fmaf(cube * x, p, __builtin_fmaf(cube_lo, 0x1.555556p-3f, 0.5f * square_lo))));
	}
	if (isnan(x))
		return x + x;
	if (x > 89.0f)
		return x * 0x1p127f;
	/* e^x is below a quarter of -1's ulp below 1: -1 */
	if (x < -18.0f)
		return -1.0f + 0x1p-100f;

	k = __builtin_fmaf(x, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;

	return __libc_fast_expm1f_kernel(__builtin_fmaf(-k, 0x1.62e430p-1f, x), k * 0x1.05c610p-29f, (int)k);
}
