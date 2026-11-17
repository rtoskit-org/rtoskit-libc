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
 * subdir.mk): a relaxed function's results within its bound to nearest (1 or 1.5 ulp; lgammaf near its zeros 2^-28
 * absolutely where looser), with no count of results not correctly rounded, a zero exactly; every other function as in ../expect.c
 */

#include <math.h>
#include <string.h>

#include "../../include/fast_contract.h"
#include "../float_accuracy.h"

double float_accuracy_tolerance(const char *name, double exact)
{
	double bound = fast_relaxed_bound_float(name);

	if (bound == 0.0 || exact == 0.0)
		return 1.0;
	/* lgammaf near its zeros: 2^-28 absolutely where that is looser than the bound (D219, D229) */
	if (strcmp(name, "lgammaf") == 0 && fabs(exact) < 0x1p-5)
		return fmax(bound, 0x1p-28 / ldexp(1.0, ilogb(exact) - 23));
	return bound;
}

bool float_accuracy_within(const char *name, double worst, long misrounded, long count)
{
	double bound = fast_relaxed_bound_float(name);

	if (bound == 0.0)
		return worst < 0.52 && misrounded <= count / 10000 + 1;
	/* lgammaf's worst in ulps is meaningless near a zero (2^-28 absolutely there): every sample met its tolerance, which decides */
	if (strcmp(name, "lgammaf") == 0)
		return true;
	return worst < bound;
}
