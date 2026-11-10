/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_expect.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIBM_EXPECT_H
#define _LIBM_EXPECT_H

/*
 * The fast contract's expectations of libm_exp, libm_trig and libm_special (PLAN D162, D163, D167; in place of
 * ../strict/libm_expect.h in the fast build type). For a relaxed function (../fast_contract.h): a reference vector
 * below its bound to nearest and of the right sign in a directed mode; an exact result F.10 does not fix the value
 * or a neighbour, and a zero exactly; a directed result of the right sign, the mode's overflow result exactly; a
 * tolerance one ulp wider; and an exact result's check no longer requires the absence of inexact. Every other
 * function keeps the strict comparisons.
 */

#include <float.h>
#include <math.h>

#include "../fast_contract.h"

static inline bool libm_vector_within(const char *name, bool single, int direction, double error, double got, double want)
{
	static const double bound[2][2] = { { 1.0, 0.5 + 0x1p-20 }, { 2.0, 1.0 } };
	double relaxed = fast_relaxed_bound_double(name);

	if (relaxed == 0.0)
		return error <= bound[direction != 0][single];
	if (direction == 0)
		return error < relaxed;
	if (isinf(want))
		return got == want;
	return isfinite(got) && !signbit(got) == !signbit(want);
}

static inline bool libm_exact(const char *name, double got, double want)
{
	if (fast_relaxed_bound_double(name) == 0.0 || want == 0.0)
		return got == want;
	return got == want || got == nextafter(want, INFINITY) || got == nextafter(want, -INFINITY);
}

static inline bool libm_exactf(const char *name, float got, float want)
{
	if (fast_relaxed_bound_double(name) == 0.0 || want == 0.0f)
		return got == want;
	return got == want || got == nextafterf(want, INFINITY) || got == nextafterf(want, -INFINITY);
}

static inline bool libm_directed(const char *name, double got, double want)
{
	if (fast_relaxed_bound_double(name) == 0.0 || isinf(want) || fabs(want) == DBL_MAX || fabs(want) == FLT_MAX)
		return got == want;
	return !isnan(got) && !signbit(got) == !signbit(want);
}

static inline bool libm_near(const char *name, double got, double want, double tolerance)
{
	if (fast_relaxed_bound_double(name) == 0.0)
		return fabs(got - want) <= tolerance;
	return fabs(got - want) <= tolerance + (nextafter(fabs(want), INFINITY) - fabs(want));
}

/* Inexact (canonical 0x10, the tests' INEXACT) may accompany a result that is no longer required exact */
static inline unsigned int libm_exact_flags(void)
{
	return ~0x10u;
}

#endif /* _LIBM_EXPECT_H */
