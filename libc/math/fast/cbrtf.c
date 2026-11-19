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
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast cbrtf of the double-FPU cores (PLAN D162, D171, D247; docs/research/libm-fast-double.md, section 41), in
 * double, where every float but zero is normal: t from the high word over 3 plus a constant fitted here (within
 * 2^-5), one Halley step t (t^3 + 2a)/(2 t^3 + a) (2^-15.5), and t (1 + r)^(1/3) for r = (a - t^3)/t^3 by the series
 * to r^2 (2^-45 relatively); rounded once to float. C23 F.10.4.1: +-0, +-inf and NaN return themselves.
 */
float cbrtf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	double a = __builtin_fabs((double)x);
	double t3;
	double t;
	double r;

	if ((bits << 1) == 0 || (bits & 0x7f800000u) == 0x7f800000u)
		return x + x;

	t = __libc_double_from((uint64_t)((uint32_t)(__libc_double_bits(a) >> 32) / 3 + 0x2a9f761cu) << 32);
	t3 = t * t * t;
	/* The quotient first: t (t^3 + 2a) would underflow near 2^-968 */
	t = t * ((t3 + 2.0 * a) / (2.0 * t3 + a));
	t3 = t * t * t;
	r = (a - t3) / t3;
	t = t + t * (r * (0x1.5555555555555p-2 - r * 0x1.c71c71c71c71cp-4));

	return (float)((bits >> 31) != 0 ? -t : t);
}
