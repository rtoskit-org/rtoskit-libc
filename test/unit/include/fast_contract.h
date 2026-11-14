/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fast_contract.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FAST_CONTRACT_H
#define _FAST_CONTRACT_H

/*
 * The fast build type's accuracy contract (PLAN D162, D163, docs/research/fast-math.md section 4), for the tests'
 * fast/ expectation files (D164, P11.02b): the relaxed families and their bound to nearest, and the error of a float
 * result in ulps. Header only and not a module: test/unit/include holds no makefile. A function not listed keeps the
 * speed flavour's contract in fast.
 */

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

/* The bound to nearest in ulps of a relaxed function, by the first length characters of its name, or 0 */
static inline double fast_relaxed_bound(const char *name, size_t length)
{
	static const struct {
		const char *name;
		double bound;
	} relaxed[] = {
		{ "exp", 1.0 },
		{ "exp2", 1.0 },
		{ "exp10", 1.0 },
		{ "expm1", 1.0 },
		{ "exp2m1", 1.0 },
		{ "exp10m1", 1.0 },
		{ "log", 1.0 },
		{ "log2", 1.0 },
		{ "log10", 1.0 },
		{ "log1p", 1.0 },
		{ "logp1", 1.0 },
		{ "log2p1", 1.0 },
		{ "log10p1", 1.0 },
		{ "pow", 1.0 },
		{ "powr", 1.0 },
		{ "pown", 1.0 },
		{ "compoundn", 1.0 },
		{ "rootn", 1.0 },
		{ "cbrt", 1.0 },
		{ "sin", 1.0 },
		{ "cos", 1.0 },
		{ "tan", 1.5 },
		{ "sinpi", 1.0 },
		{ "cospi", 1.0 },
		{ "tanpi", 1.0 },
		{ "asin", 1.0 },
		{ "acos", 1.0 },
		{ "atan", 1.0 },
		{ "atan2", 1.0 },
		{ "asinpi", 1.0 },
		{ "acospi", 1.0 },
		{ "atanpi", 1.0 },
		{ "atan2pi", 1.0 },
		{ "sinh", 1.5 },
		{ "cosh", 1.5 },
		{ "tanh", 1.5 },
		{ "asinh", 1.5 },
		{ "acosh", 1.5 },
		{ "atanh", 1.5 },
		{ "erf", 1.5 },
	};

	for (size_t i = 0; i < sizeof(relaxed) / sizeof(relaxed[0]); ++i)
		if (strlen(relaxed[i].name) == length && strncmp(name, relaxed[i].name, length) == 0)
			return relaxed[i].bound;
	return 0.0;
}

/* The same by a double function's name (sin) */
static inline double fast_relaxed_bound_double(const char *name)
{
	return fast_relaxed_bound(name, strlen(name));
}

/* The same by a float function's name (sinf), its suffix dropped */
static inline double fast_relaxed_bound_float(const char *name)
{
	return fast_relaxed_bound(name, strlen(name) - 1);
}

/*
 * The error of a float result against a value good to far below a float ulp (the double function), in ulps of
 * the value's binade (2^-149 below the normal range); infinite where a NaN, an infinity or the sign differs
 */
static inline double fast_float_error(float got, double exact)
{
	int exponent;

	if (isnan(exact))
		return isnan(got) ? 0.0 : INFINITY;
	/* Past FLT_MAX the float result is infinite */
	if (isinf(exact) || isinf(got))
		return isinf(got) && (isinf(exact) || fabs(exact) > FLT_MAX) && !signbit(got) == !signbit(exact) ? 0.0 : INFINITY;
	if (isnan(got) || !signbit(got) != !signbit(exact))
		return INFINITY;
	frexp(fabs(exact), &exponent);
	if (exponent < FLT_MIN_EXP)
		exponent = FLT_MIN_EXP;
	return fabs((double)got - exact) / ldexp(1.0, exponent - FLT_MANT_DIG);
}

#endif /* _FAST_CONTRACT_H */
