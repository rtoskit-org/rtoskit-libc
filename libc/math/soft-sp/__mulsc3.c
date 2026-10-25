/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __mulsc3.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <complex.h>
#include <math.h>

#include "softfloat.h"

/*
 * (a + bi)(c + di), the compiler's helper for float complex multiplication
 * (C23 6.5.6, Annex G.5.2; PLAN D100, D109): soft-dp's __muldc3 in float.
 * The textbook formula, and when both parts come out NaN, the recovery of
 * G.5.2p1: an infinite operand times a nonzero finite or infinite one is
 * an infinity. libgcc's version would bring libgcc's float helpers with it
 * on armv7-m, where the soft-sp layer has its own.
 */
float complex __mulsc3(float a, float b, float c, float d)
{
	float ac = a * c;
	float bd = b * d;
	float ad = a * d;
	float bc = b * c;
	float x = ac - bd;
	float y = ad + bc;
	bool recalculate = false;

	if (!isnan(x) || !isnan(y))
		return CMPLXF(x, y);

	if (isinf(a) || isinf(b)) {
		a = __builtin_copysignf(isinf(a) ? 1.0f : 0.0f, a);
		b = __builtin_copysignf(isinf(b) ? 1.0f : 0.0f, b);
		if (isnan(c))
			c = __builtin_copysignf(0.0f, c);
		if (isnan(d))
			d = __builtin_copysignf(0.0f, d);
		recalculate = true;
	}
	if (isinf(c) || isinf(d)) {
		c = __builtin_copysignf(isinf(c) ? 1.0f : 0.0f, c);
		d = __builtin_copysignf(isinf(d) ? 1.0f : 0.0f, d);
		if (isnan(a))
			a = __builtin_copysignf(0.0f, a);
		if (isnan(b))
			b = __builtin_copysignf(0.0f, b);
		recalculate = true;
	}
	if (!recalculate && (isinf(ac) || isinf(bd) || isinf(ad) || isinf(bc))) {
		if (isnan(a))
			a = __builtin_copysignf(0.0f, a);
		if (isnan(b))
			b = __builtin_copysignf(0.0f, b);
		if (isnan(c))
			c = __builtin_copysignf(0.0f, c);
		if (isnan(d))
			d = __builtin_copysignf(0.0f, d);
		recalculate = true;
	}
	if (recalculate) {
		x = INFINITY * (a * c - b * d);
		y = INFINITY * (a * d + b * c);
	}

	return CMPLXF(x, y);
}
