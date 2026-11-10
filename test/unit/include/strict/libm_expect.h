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
 * The strict contract's expectations of libm_exp, libm_trig and libm_special (debug, release, speed; PLAN D47, D57,
 * D98; P11.02c): exactly the comparisons those tests made before. ../fast/libm_expect.h has the fast contract's; the
 * modules choose one by include path on LIBC_FLAVOURS. A name is the function's double name (exp2 for exp2f and
 * exp2l), which the fast contract looks up.
 */

#include <math.h>

/* A reference vector's error: at most 1 ulp for double, correctly rounded float, to nearest; 2 and 1 ulp directed */
static inline bool libm_vector_within([[maybe_unused]] const char *name, bool single, int direction, double error, [[maybe_unused]] double got, [[maybe_unused]] double want)
{
	static const double bound[2][2] = { { 1.0, 0.5 + 0x1p-20 }, { 2.0, 1.0 } };

	return error <= bound[direction != 0][single];
}

/* An exact result to nearest (exp2(k), log2(2^k), cbrt(27)): the value itself */
static inline bool libm_exact(const char *, double got, double want)
{
	return got == want;
}

static inline bool libm_exactf(const char *, float got, float want)
{
	return got == want;
}

/* The correctly rounded result in a directed mode */
static inline bool libm_directed(const char *, double got, double want)
{
	return got == want;
}

/* A result within an absolute tolerance of a reference */
static inline bool libm_near(const char *, double got, double want, double tolerance)
{
	return fabs(got - want) <= tolerance;
}

/* The exceptions an exact result's check compares (canonical FE_* bits, as the tests' own): all of them */
static inline unsigned int libm_exact_flags(void)
{
	return ~0u;
}

#endif /* _LIBM_EXPECT_H */
