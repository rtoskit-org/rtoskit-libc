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
 * first zero, and the overflow and underflow exceptions where the core keeps them (D44, D45); for powf, every case
 * of C23 F.10.4.5 as well. Each point is far enough from its threshold, in ulps, that every contract gives the same
 * answer.
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

static void check_logarithms(void)
{
	static float (*const functions[])(float) = { logf, log2f, log10f };

	for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		float (*f)(float) = functions[i];

		/* F.10.3.7, .8, .10: NaN; +inf; log(+-0) = -inf with divide-by-zero; a negative x is invalid */
		QUIET(isnan(f(NAN)) && f(INFINITY) == INFINITY);
		RAISES(f(0.0f) == -INFINITY && f(-0.0f) == -INFINITY, FE_DIVBYZERO);
		RAISES(isnan(f(-1.0f)) && isnan(f(-0x1p-149f)) && isnan(f(-INFINITY)), FE_INVALID);
		/* log(1) = +0 (regression: rounding downward, the fast double path's m - 1 gave -0) */
		QUIET(is_positive_zero(f(1.0f)));
#ifdef FE_DOWNWARD
		fesetround(FE_DOWNWARD);
		QUIET(is_positive_zero(f(1.0f)));
		fesetround(FE_TONEAREST);
#endif
		/* The ends of the range: the smallest subnormal and FLT_MAX, finite and of the right sign */
		QUIET(f(0x1p-149f) < 0.0f && isfinite(f(0x1p-149f)) && f(FLT_MAX) > 0.0f && isfinite(f(FLT_MAX)));
	}
	/* log2 is exact at the powers of two, normal and subnormal */
	for (int k = -149; k <= 127; k += 7)
		QUIET(log2f(ldexpf(1.0f, k)) == (float)k);
	CHECK(logf(0x1p-149f) < -103.27f && logf(0x1p-149f) > -103.28f && logf(FLT_MAX) > 88.72f && logf(FLT_MAX) < 88.73f);
}

static bool is_negative_zero(float x)
{
	return x == 0.0f && signbit(x);
}

static void check_powers(void)
{
	/* F.10.4.5: pow(x, +-0) = 1 and pow(+1, y) = 1 even for a NaN, pow(-1, +-inf) = 1, all without an exception */
	QUIET(is_one(powf(NAN, 0.0f)) && is_one(powf(INFINITY, -0.0f)) && is_one(powf(1.0f, NAN)) && is_one(powf(1.0f, -INFINITY)));
	QUIET(is_one(powf(-1.0f, INFINITY)) && is_one(powf(-1.0f, -INFINITY)) && is_one(powf(-0.0f, 0.0f)));
	QUIET(isnan(powf(NAN, 2.0f)) && isnan(powf(2.0f, NAN)) && isnan(powf(-1.0f, NAN)));

	/* pow(+-0, y): +-inf with divide-by-zero for a negative odd integer y, +inf for another negative y */
	RAISES(powf(0.0f, -1.0f) == INFINITY && powf(-0.0f, -1.0f) == -INFINITY && powf(-0.0f, -3.0f) == -INFINITY, FE_DIVBYZERO);
	RAISES(powf(-0.0f, -2.0f) == INFINITY && powf(-0.0f, -0.5f) == INFINITY && powf(0.0f, -0x1p30f) == INFINITY, FE_DIVBYZERO);
	CHECK(powf(0.0f, -INFINITY) == INFINITY && powf(-0.0f, -INFINITY) == INFINITY);
	/* ... and +-0 for a positive odd integer y, +0 for another positive y */
	QUIET(is_negative_zero(powf(-0.0f, 3.0f)) && is_positive_zero(powf(0.0f, 3.0f)) && is_positive_zero(powf(-0.0f, 2.0f)));
	QUIET(is_positive_zero(powf(-0.0f, 0.5f)) && is_positive_zero(powf(-0.0f, 0x1p30f)) && is_positive_zero(powf(-0.0f, INFINITY)));

	/* pow(x, +-inf) by |x| against 1 */
	QUIET(powf(0.5f, -INFINITY) == INFINITY && is_positive_zero(powf(-2.0f, -INFINITY)));
	QUIET(is_positive_zero(powf(-0.5f, INFINITY)) && powf(2.0f, INFINITY) == INFINITY);
	/* pow(-inf, y) and pow(+inf, y) by y's sign and parity */
	QUIET(is_negative_zero(powf(-INFINITY, -3.0f)) && is_positive_zero(powf(-INFINITY, -2.0f)) && is_positive_zero(powf(-INFINITY, -0.5f)));
	QUIET(powf(-INFINITY, 3.0f) == -INFINITY && powf(-INFINITY, 2.0f) == INFINITY && powf(-INFINITY, 0.5f) == INFINITY);
	QUIET(is_positive_zero(powf(INFINITY, -0.5f)) && powf(INFINITY, 0x1p-20f) == INFINITY);

	/* A negative finite x to a finite non-integer power is invalid; every |y| from 2^24 up is an even integer */
	RAISES(isnan(powf(-2.0f, 0.5f)) && isnan(powf(-0x1p-149f, 1.5f)) && isnan(powf(-FLT_MAX, -0x1.fffffep22f)), FE_INVALID);
	QUIET(powf(-1.0f, 0x1p24f) == 1.0f && powf(-1.0f, 0x1.fffffep23f) == -1.0f && powf(-1.0f, -0x1p30f) == 1.0f);
	QUIET(powf(-2.0f, 3.0f) < -7.99f && powf(-2.0f, 3.0f) > -8.01f && powf(-2.0f, -2.0f) > 0.2499f && powf(-2.0f, -2.0f) < 0.2501f);

	/* The first overflow (2^128) and finite results below it, both signs */
	RAISES(powf(2.0f, 128.0f) == INFINITY && powf(-2.0f, 129.0f) == -INFINITY && powf(-2.0f, 130.0f) == INFINITY, FE_OVERFLOW);
	RAISES(powf(0.5f, -200.0f) == INFINITY && powf(FLT_MAX, 2.0f) == INFINITY && powf(1.0001f, 1e7f) == INFINITY, FE_OVERFLOW);
	QUIET(isfinite(powf(2.0f, 127.99f)) && powf(2.0f, 127.99f) > 0x1p127f && powf(-2.0f, 127.0f) < -0x1.fffffcp126f);
	QUIET(isfinite(powf(FLT_MAX, 1.0f)) && powf(FLT_MAX, 0.5f) >= 0x1.fffffcp63f && powf(FLT_MAX, 0.5f) <= 0x1p64f);

	/* Gradual underflow: subnormal results, the last nonzero (above 2^-150) and the first zeros, both signs */
	CHECK(powf(2.0f, -126.0f) >= 0x1.fffffcp-127f && powf(2.0f, -126.0f) < 0x1.000004p-126f && powf(-2.0f, -149.0f) < 0.0f);
	RAISES(is_subnormal(powf(2.0f, -140.5f)) && is_subnormal(powf(0.5f, 140.0f)) && is_subnormal(powf(FLT_MIN, 1.1f)), FE_UNDERFLOW);
	RAISES(powf(2.0f, -149.4f) == 0x1p-149f && powf(-0x1.4p-50f, 3.0f) == -0x1p-149f, FE_UNDERFLOW);
	RAISES(is_positive_zero(powf(2.0f, -151.0f)) && is_negative_zero(powf(-2.0f, -151.0f)) && is_positive_zero(powf(0.5f, 1000.0f)), FE_UNDERFLOW);
	RAISES(is_positive_zero(powf(FLT_MIN, 2.0f)) && is_positive_zero(powf(0x1p-149f, 1.5f)) && is_positive_zero(powf(1.0001f, -1e7f)), FE_UNDERFLOW);
}

int main(void)
{
	check_exponentials();
	check_logarithms();
	check_powers();

	if (failures == 0)
		puts("pass: libm_thresholds");
	return failures == 0 ? 0 : 1;
}
