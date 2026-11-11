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

/* The argument whose double result is a float halfway point while the exact value is not, as libc/math/erfcf.c (PLAN D98) */
static const struct __libc_float_case cases[] = {
	{ 0xb76c9f62, 0x3f800085 }, /* -0x1.d93ec4p-17 -> 0x1.00010ap+0 */
};

/*
 * The fast erfcf of the double-FPU cores (PLAN D162, D178): erfcf keeps the strict contract (correctly rounded, in every
 * rounding direction), so the fast result is used only where a rounding test proves it the correctly rounded one. Below
 * 1/2 in magnitude 1 - x P(x^2), from 1/2 e^(-x^2) R(x) (__libc_fast_erfc_double), 2 minus it below -1/2, all in double
 * to 2^-34; that is correctly rounded in every direction unless its 29 bits below the float's lie within 2^20 of a float
 * or a halfway point (2^-33 relative, about 0.8 % of arguments), where, and beyond -3.9 or 9.19 (results within a
 * quarter ulp of 2, or subnormal), the other build types' double erfc decides (the trigonometric note, section 12).
 */
float erfcf(float x)
{
	double d = x;
	double a = __builtin_fabs(d);
	double r;
	uint32_t low;

	/* NaN (compared quietly), the infinities, the ends: as the other build types */
	if (!(isgreaterequal(x, -3.9f) && islessequal(x, 9.19f)))
		return __libc_float_round(erfc(d), x, cases, sizeof(cases) / sizeof(cases[0]));
	if (a < 0.5) {
		r = 1.0 - __libc_fast_erf_small(d);
	} else {
		r = __libc_fast_erfc_double(a);
		if (x < 0)
			r = 2.0 - r;
	}
	/* The rounding test: the low 29 bits against a float (0 or 2^29) and the halfway point (2^28), 2^20 either side */
	low = (uint32_t)__libc_double_bits(r) & 0x1fffffff;
	if (((low + 0x00100000) & 0x0fffffff) < 0x00200000)
		return __libc_float_round(erfc(d), x, cases, sizeof(cases) / sizeof(cases[0]));

	return (float)r;
}
