/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_thresholds.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The special values and range thresholds every build type keeps (docs/research/fast-math.md sections 4.1 and 8;
 * PLAN D162, D163): what the fast contract leaves exact, checked in debug, release, speed and fast alike. Per
 * function: the C23 F.10 values (NaN, both infinities, both zeros), the first overflow to +inf and the last finite
 * results, the smallest normal and a subnormal result (gradual underflow, no flush), the last nonzero result and the
 * first zero, and the overflow and underflow exceptions where the core keeps them (D44, D45). Each point is far
 * enough from its threshold, in ulps, that every contract gives the same answer.
 */

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

static int failures;

static void fail(int line, const char *what)
{
	printf("fail: line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

/* The condition holds, and the exception (an FE_* name) is raised where the core keeps exceptions (C23 7.6p6) */
#ifdef FE_OVERFLOW
#define RAISES(condition, exception)                                  \
	do {                                                              \
		feclearexcept(FE_ALL_EXCEPT);                                 \
		CHECK(condition);                                             \
		if (!fetestexcept(exception))                                 \
			fail(__LINE__, #condition " does not raise " #exception); \
	} while (false)
/* ... and none of invalid, divide-by-zero, overflow and underflow is */
#define QUIET(condition)                                                          \
	do {                                                                          \
		feclearexcept(FE_ALL_EXCEPT);                                             \
		CHECK(condition);                                                         \
		if (fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW)) \
			fail(__LINE__, #condition " raises an exception");                    \
	} while (false)
#else
#define RAISES(condition, exception) CHECK(condition)
#define QUIET(condition) CHECK(condition)
#endif

static bool is_one(float x)
{
	return x == 1.0f;
}

static bool is_positive_zero(float x)
{
	return x == 0.0f && !signbit(x);
}

/* A positive subnormal result: nonzero and below FLT_MIN (no flush to zero) */
static bool is_subnormal(float x)
{
	return x > 0.0f && x < FLT_MIN;
}

static void check_exponentials(void)
{
	/* F.10.3.1-3: NaN, e^+inf = +inf, e^-inf = +0, e^+-0 = 1, all without an exception */
	QUIET(isnan(expf(NAN)) && isnan(exp2f(NAN)) && isnan(exp10f(NAN)));
	QUIET(expf(INFINITY) == INFINITY && exp2f(INFINITY) == INFINITY && exp10f(INFINITY) == INFINITY);
	QUIET(is_positive_zero(expf(-INFINITY)) && is_positive_zero(exp2f(-INFINITY)) && is_positive_zero(exp10f(-INFINITY)));
	QUIET(is_one(expf(0.0f)) && is_one(expf(-0.0f)) && is_one(exp2f(-0.0f)) && is_one(exp10f(0.0f)));

	/* The first overflow (ln, log2 and log10 of FLT_MAX are 88.72, 128 and 38.53) and finite results below it */
	RAISES(expf(88.8f) == INFINITY && expf(1000.0f) == INFINITY, FE_OVERFLOW);
	RAISES(exp2f(128.0f) == INFINITY && exp2f(200.5f) == INFINITY, FE_OVERFLOW);
	RAISES(exp10f(38.6f) == INFINITY && exp10f(100.0f) == INFINITY, FE_OVERFLOW);
	QUIET(isfinite(expf(88.7f)) && expf(88.7f) > 0x1p127f && isfinite(exp10f(38.5f)) && exp10f(38.5f) > 0x1p127f);
	QUIET(exp2f(127.0f) == 0x1p127f && isfinite(exp2f(127.99f)) && exp2f(127.99f) > 0x1p127f);

	/* The smallest normal results, then subnormal ones: gradual underflow, exact at exp2f's integers */
	QUIET(exp2f(-126.0f) == FLT_MIN && expf(-87.0f) > FLT_MIN && exp10f(-37.5f) > FLT_MIN);
	CHECK(exp2f(-140.0f) == 0x1p-140f && exp2f(-149.0f) == 0x1p-149f);
	RAISES(is_subnormal(expf(-95.0f)) && is_subnormal(exp2f(-140.5f)) && is_subnormal(exp10f(-41.0f)), FE_UNDERFLOW);

	/* The last nonzero results (above half the smallest subnormal, 2^-150) and the first zeros */
	RAISES(expf(-103.5f) == 0x1p-149f && exp10f(-44.9f) == 0x1p-149f && exp2f(-149.4f) == 0x1p-149f, FE_UNDERFLOW);
	RAISES(is_positive_zero(expf(-104.5f)) && is_positive_zero(exp2f(-151.0f)) && is_positive_zero(exp10f(-45.5f)), FE_UNDERFLOW);
	RAISES(is_positive_zero(expf(-1000.0f)) && is_positive_zero(exp2f(-1000.0f)) && is_positive_zero(exp10f(-1000.0f)), FE_UNDERFLOW);
}

int main(void)
{
	check_exponentials();

	if (failures == 0)
		puts("pass: libm_thresholds");
	return failures == 0 ? 0 : 1;
}
