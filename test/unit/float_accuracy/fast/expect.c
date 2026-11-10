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
 * The accuracy of the float functions in the fast build type (PLAN D162, D163, D164; in place of ../expect.c,
 * subdir.mk): a relaxed function's results within its bound to nearest (1 or 1.5 ulp), with no count of results
 * not correctly rounded, a zero exactly; every other function as in ../expect.c
 */

#include "../../include/fast_contract.h"
#include "../float_accuracy.h"

double float_accuracy_tolerance(const char *name, double exact)
{
	double bound = fast_relaxed_bound_float(name);

	return bound == 0.0 || exact == 0.0 ? 1.0 : bound;
}

bool float_accuracy_within(const char *name, double worst, long misrounded, long count)
{
	double bound = fast_relaxed_bound_float(name);

	if (bound == 0.0)
		return worst < 0.52 && misrounded <= count / 10000 + 1;
	return worst < bound;
}
