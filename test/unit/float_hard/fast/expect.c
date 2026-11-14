/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * expect.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The hard cases' expectation in the fast build type (PLAN D162, D163, D164; in place of ../expect.c, subdir.mk).
 * A relaxed function to nearest may return either float around the exact value, which lies within half an ulp of
 * result toward direction (below 1 ulp), and with a 1.5 ulp bound also the float on the other side of result; in a
 * directed mode it promises no bound, only a finite result of the correctly rounded one's sign. The other functions
 * (erfcf, lgammaf, tgammaf) keep the correctly rounded result in every mode.
 */

#include <math.h>
#include <string.h>

#include "../../include/fast_contract.h"
#include "../float_hard.h"

bool float_hard_accept(const char *name, bool nearest, float got, float want, float result, int direction)
{
	double bound = fast_relaxed_bound_float(name);

	if (bound == 0.0)
		return memcmp(&got, &want, sizeof(got)) == 0;
	if (!nearest)
		return isfinite(got) && !signbit(got) == !signbit(want);
	return got == result || got == nextafterf(result, direction > 0 ? INFINITY : -INFINITY) || (bound > 1.0 && got == nextafterf(result, direction > 0 ? -INFINITY : INFINITY));
}
