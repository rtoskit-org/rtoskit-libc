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

#include "internal/math-fast.h"

/*
 * The other build types' erfcf of this layer (libc/math/sp-nearest/erfcf.c, PLAN D63), the fallback of the rounding test
 * below; this file replaces that one in the fast build type, so it keeps its own copy
 */
static float erfcf_float_float(float x)
{
	struct __libc_ff value;
	struct __libc_ff square;
	struct __libc_ff fraction;
	int levels;
	int exponent;

	if (isnan(x))
		return x + x;
	if (x < -3.9f)
		return 2.0f;
	if (x > 10.06f)
		return 0x1p-100f * 0x1p-100f;
	/* erfc(x) = 1 - 2x/sqrt(pi) rounds to 1 - x */
	if (fabsf(x) < 0x1p-26f)
		return 1.0f - x;

	if (x < 1.0f) {
		value = __libc_erff_core(fabsf(x));
		if (x > 0)
			value = (struct __libc_ff){ .hi = -value.hi, .lo = -value.lo };
		value = __libc_ff_add((struct __libc_ff){ .hi = 1.0f, .lo = 0.0f }, value);
		return value.hi + value.lo;
	}

	levels = (int)(150.0f / (x * x)) + 10;
	fraction = (struct __libc_ff){ .hi = x, .lo = 0.0f };
	for (int k = levels; k > 0; --k)
		fraction = __libc_ff_add((struct __libc_ff){ .hi = x, .lo = 0.0f }, __libc_ff_divide((struct __libc_ff){ .hi = 0.5f * (float)k, .lo = 0.0f }, fraction));

	square = __libc_ff_product(x, x);
	value = __libc_exp2f_core(__libc_ff_multiply((struct __libc_ff){ .hi = -square.hi, .lo = -square.lo }, __LIBC_INV_LN2F), &exponent);
	value = __libc_ff_divide(__libc_ff_multiply(value, __LIBC_INV_SQRTPIF), fraction);

	return __libc_exp2f_scale(value, exponent);
}

/*
 * The fast erfcf of the single-precision FPU cores (PLAN D162, D178, D179): erfcf keeps the strict contract (correctly
 * rounded; this layer rounds to nearest only, D57), so the fast result is used only where a rounding test proves it the
 * correctly rounded one. erfc |x| = 2^k (hi + lo) to about 2^-32 (__libc_fast_erfcf_accurate), 2 minus it for a negative
 * x; the pair rounded once, and kept unless the rounding's remainder lies within 2^-31 of the result from the halfway
 * point (about 1.6 % of arguments), where, and beyond -3.9 and 9.19 (results within a quarter ulp of 2, or subnormal),
 * the float-float code decides (the trigonometric note, section 15).
 */
float erfcf(float x)
{
	float a = __builtin_fabsf(x);
	float hi;
	float lo;
	float s;
	float f;
	float d;
	float half;
	uint32_t bits;
	int k;

	/* NaN (compared quietly), the infinities, the ends */
	if (!(isgreaterequal(x, -3.9f) && islessequal(x, 9.19f)))
		return erfcf_float_float(x);
	/* erfc(x) = 1 - 2x/sqrt(pi) rounds to 1 - x */
	if (a < 0x1p-26f)
		return 1.0f - x;
	hi = __libc_fast_erfcf_accurate(a, &k, &lo);
	if (x < 0) {
		/* k >= -23 here: 2^k scales both parts exactly, and 2 - erfc|x| is at least 1 */
		s = __libc_float_from((uint32_t)(k + 127) << 23);
		hi = __libc_fast_two_sum(2.0f, -(hi * s), &d);
		lo = d - lo * s;
		k = 0;
	}
	/*
	 * f = hi + lo rounded and d its exact remainder; the exact value is within 2^-32.3 of hi + lo (measured over every
	 * argument), so f is the correctly rounded result unless |d| is within 2^-31 f of the halfway point: half an ulp of f
	 * away, or a quarter below a power of two, where the ulp below is half the ulp above
	 */
	f = __libc_fast_two_sum(hi, lo, &d);
	bits = __libc_float_bits(f);
	half = __libc_float_from((bits & 0x7f800000) - ((bits & 0x007fffff) == 0 && d < 0 ? 25u << 23 : 24u << 23));
	if (half - __builtin_fabsf(d) <= f * 0x1p-31f)
		return erfcf_float_float(x);
	/* k >= -122 here: 2^k is normal and the result too, so the scaling is exact */
	return f * __libc_float_from((uint32_t)(k + 127) << 23);
}
