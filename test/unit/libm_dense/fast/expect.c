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
 * The dense sweeps' and near-halfway cases' expectation in the fast build type (PLAN D162, D163, D164; in place of
 * ../expect.c, subdir.mk): a relaxed float function's results within its bound of the double function, which is
 * good to far below a float ulp; a relaxed double function's near-halfway result the correctly rounded one or a
 * neighbour (the reference here is the correctly rounded value, so this is its bound plus half an ulp); erfcf as in
 * ../expect.c
 */

#include <math.h>
#include <string.h>

#include "../../include/fast_contract.h"
#include "../libm_dense.h"

bool libm_dense_accept(const char *name, float got, double reference)
{
	double bound = fast_relaxed_bound_float(name);
	float want = (float)reference;

	if (bound == 0.0)
		return memcmp(&got, &want, sizeof(got)) == 0;
	return fast_float_error(got, reference) < bound;
}

unsigned long libm_dense_allowance([[maybe_unused]] const char *name, unsigned long count)
{
	return count / 10000 + 1;
}

bool libm_dense_near(const char *name, float got, float want, double reference)
{
	if (fast_relaxed_bound_float(name) == 0.0)
		return memcmp(&got, &want, sizeof(got)) == 0;
	return fast_float_error(got, reference) < fast_relaxed_bound_float(name);
}

bool libm_dense_near_double(const char *name, double got, double want)
{
	if (fast_relaxed_bound_double(name) == 0.0)
		return memcmp(&got, &want, sizeof(got)) == 0;
	return got == want || got == nextafter(want, INFINITY) || got == nextafter(want, -INFINITY);
}
