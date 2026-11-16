/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * erfcf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The ends (C23 F.10.5.2): NaN itself; erfc(+inf) is +0 and erfc(-inf) 2, exactly; below -3.9 erfc is within a quarter
 * ulp of 2 (2 with inexact); above 10.06 below half the smallest subnormal (+0 with underflow)
 */
[[gnu::noinline]] static float erfcf_ends(float x)
{
	if (isnan(x))
		return x + x;
	if (x == INFINITY)
		return 0.0f;
	if (x == -INFINITY)
		return 2.0f;
	if (x < 0)
		return 2.0f - 0x1p-26f;

	return 0x1p-100f * 0x1p-100f;
}

/*
 * The fast erfcf of the single-precision FPU cores (PLAN D162, D219: 1 ulp to nearest, no longer correctly rounded as
 * D178 kept it): below 1/2, 1 - erf x with erf x = (2/sqrt(pi)) x + x^3 Q(x^2) as in erff, its leading product exact
 * by a fused multiply-add and 1 minus it by a two-sum; above, erfc|x| = 2^k (hi + lo) from the pair kernel
 * (__libc_fast_erfcf_kernel), 2 minus it for a negative x, and a subnormal result scaled in two steps (one rounding
 * after hi + lo's). docs/research/libm-fast-float-trig.md, section 23. Round to nearest only, as the layer (D57).
 */
float erfcf(float x)
{
	float a = __builtin_fabsf(x);
	float hi;
	float lo;
	float z;
	float p;
	float rest;
	float error;
	int k;

	/* NaN (compared quietly), the infinities, the ends */
	if (!(isgreaterequal(x, -3.9f) && islessequal(x, 10.06f)))
		return erfcf_ends(x);
	/* erfc(x) = 1 - 2x/sqrt(pi) rounds to 1 - x */
	if (a < 0x1p-26f)
		return 1.0f - x;
	if (a < 0.5f) {
		z = x * x;
		p = 0x1.20dd76p+0f * x;
		rest = __builtin_fmaf(0x1.20dd76p+0f, x, -p) + __builtin_fmaf(-0x1.f9a0e6p-25f, x, x * z * __libc_fast_erff_tail(z));
		hi = __libc_fast_two_sum(1.0f, -p, &error);
		return hi + (error - rest);
	}
	hi = __libc_fast_erfcf_kernel(a, &k, &lo);
	if (x < 0) {
		/* k >= -23 here: 2^k scales both parts exactly, and 2 - erfc|x| is at least 1 */
		z = __libc_float_from((uint32_t)(k + 127) << 23);
		hi = __libc_fast_two_sum(2.0f, -(hi * z), &error);
		return hi + (error - lo * z);
	}
	/* Down to 2^-126 one exact scaling; below, hi + lo scaled to normal first, then the one rounding into the subnormals */
	if (k >= -126)
		return (hi + lo) * __libc_float_from((uint32_t)(k + 127) << 23);

	return (hi + lo) * __libc_float_from((uint32_t)(k + 64 + 127) << 23) * 0x1p-64f;
}
