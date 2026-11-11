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
 * of C23 F.10.4.5 as well; the m1 and p1 forms and cbrtf near 0 too, where the relative accuracy matters. Each point is far enough from its threshold, in ulps, that every contract gives the same
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

/* x within 2^-21 of a nonzero want, relatively: a ratio, so that no step underflows near FLT_MIN */
static bool near(float x, float want)
{
	return fabsf(x / want - 1.0f) <= 0x1p-21f;
}

static void check_minus_one(void)
{
	static float (*const functions[])(float) = { expm1f, exp2m1f, exp10m1f };

	for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		float (*f)(float) = functions[i];

		/* F.10.3.3, .5, .6: NaN, +inf, -1 for -inf, +-0 with its sign, all without an exception */
		QUIET(isnan(f(NAN)) && f(INFINITY) == INFINITY && f(-INFINITY) == -1.0f);
		QUIET(is_positive_zero(f(0.0f)) && is_negative_zero(f(-0.0f)));
		/* Far below 0 the result is -1; beside it, still above */
		CHECK(f(-1000.0f) == -1.0f && f(-30.0f) == -1.0f);
	}
	/* The first overflow (ln, log2 and log10 of FLT_MAX are 88.72, 128 and 38.53) and finite results below it */
	RAISES(expm1f(88.8f) == INFINITY && exp2m1f(128.0f) == INFINITY && exp10m1f(38.6f) == INFINITY, FE_OVERFLOW);
	RAISES(expm1f(1000.0f) == INFINITY && exp2m1f(200.5f) == INFINITY && exp10m1f(100.0f) == INFINITY, FE_OVERFLOW);
	QUIET(isfinite(expm1f(88.7f)) && expm1f(88.7f) > 0x1p127f && isfinite(exp10m1f(38.5f)) && exp10m1f(38.5f) > 0x1p127f);
	QUIET(isfinite(exp2m1f(127.99f)) && exp2m1f(127.99f) > 0x1p127f && near(exp2m1f(127.0f), 0x1p127f));
	/* -1 from where the power is below a quarter of -1's ulp below 1, and not yet above it */
	CHECK(expm1f(-18.5f) == -1.0f && exp2m1f(-26.5f) == -1.0f && exp10m1f(-8.5f) == -1.0f);
	CHECK(expm1f(-16.0f) > -1.0f && exp2m1f(-23.0f) > -1.0f && exp10m1f(-7.0f) > -1.0f);
	/* Near 0 the relative accuracy: x, x ln2 and x ln10, subnormal results included */
	QUIET(expm1f(0x1p-30f) == 0x1p-30f && expm1f(-0x1p-30f) == -0x1p-30f && near(expm1f(0x1p-12f), 0x1.0008p-12f));
	CHECK(near(exp2m1f(0x1p-60f), 0x1.62e430p-61f) && near(exp10m1f(-0x1p-60f), -0x1.26bb1cp-59f));
	CHECK(is_subnormal(exp2m1f(0x1p-140f)) && is_subnormal(exp10m1f(0x1p-140f)) && is_subnormal(expm1f(0x1p-140f)));
	QUIET(near(expm1f(1.0f), 0x1.b7e152p+0f) && near(exp2m1f(-1.0f), -0.5f) && near(exp10m1f(2.0f), 99.0f));
}

static void check_plus_one(void)
{
	static float (*const functions[])(float) = { log1pf, log2p1f, log10p1f };

	for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		float (*f)(float) = functions[i];

		/* F.10.3.8, .9, .11: NaN; +inf; log(1 + -1) = -inf with divide-by-zero; below -1 invalid; +-0 with its sign */
		QUIET(isnan(f(NAN)) && f(INFINITY) == INFINITY);
		RAISES(f(-1.0f) == -INFINITY, FE_DIVBYZERO);
		RAISES(isnan(f(-1.5f)) && isnan(f(-0x1.000002p+0f)) && isnan(f(-INFINITY)), FE_INVALID);
		QUIET(is_positive_zero(f(0.0f)) && is_negative_zero(f(-0.0f)));
		/* The ends of the range: just above -1, and FLT_MAX */
		QUIET(f(-0x1.fffffep-1f) < -1.0f && isfinite(f(-0x1.fffffep-1f)) && f(FLT_MAX) > 1.0f && isfinite(f(FLT_MAX)));
	}
	/* Near 0 the relative accuracy: x, x/ln2 and x/ln10, subnormal results included */
	QUIET(log1pf(0x1p-30f) == 0x1p-30f && log1pf(-0x1p-30f) == -0x1p-30f && near(log1pf(0x1p-12f), 0x1.fffp-13f));
	CHECK(near(log2p1f(0x1p-60f), 0x1.715476p-60f) && near(log10p1f(-0x1p-60f), -0x1.bcb7b2p-62f));
	CHECK(is_subnormal(log2p1f(0x1p-140f)) && is_subnormal(log10p1f(0x1p-140f)) && is_subnormal(log1pf(0x1p-140f)));
	QUIET(near(log1pf(1.0f), 0x1.62e430p-1f) && near(log2p1f(1.0f), 1.0f) && near(log10p1f(99.0f), 2.0f));
	CHECK(near(log1pf(-0x1.fffffep-1f), -0x1.0a2b24p+4f) && near(log2p1f(FLT_MAX), 128.0f) && near(log10p1f(FLT_MAX), 0x1.344136p+5f));
}

static void check_cube_root(void)
{
	/* F.10.4.1: +-0, +-inf and NaN unchanged, without an exception */
	QUIET(is_positive_zero(cbrtf(0.0f)) && is_negative_zero(cbrtf(-0.0f)) && isnan(cbrtf(NAN)));
	QUIET(cbrtf(INFINITY) == INFINITY && cbrtf(-INFINITY) == -INFINITY);
	/* Cubes, the ends of the range (a subnormal x has a normal root) and the sign */
	QUIET(near(cbrtf(27.0f), 3.0f) && near(cbrtf(-8.0f), -2.0f) && near(cbrtf(0x1p-147f), 0x1p-49f) && near(cbrtf(-0x1p-149f), -0x1.428a30p-50f));
	QUIET(near(cbrtf(FLT_MAX), 0x1.965feap+42f) && near(cbrtf(FLT_MIN), 0x1p-42f) && near(cbrtf(-0x1.fffffep-127f), -0x1p-42f));
}

/* Each side of where the cores without an FPU change path (P11.03e: |x| = 87, 126, 37.5 for float-float) */
static void check_fixed_edges(void)
{
	QUIET(near(expf(0x1.5bfffep+6f), 0x1.6daf5ap+125f) && near(expf(0x1.5c0002p+6f), 0x1.6db0c8p+125f) && near(expf(-0x1.5bfffep+6f), 0x1.666dc0p-126f));
	QUIET(near(exp2f(0x1.f7fffep+6f), 0x1.ffff4ep+125f) && near(exp2f(0x1.f80002p+6f), 0x1.000058p+126f));
	QUIET(near(exp10f(0x1.2bfffep+5f), 0x1.7ca45ap+124f) && near(exp10f(0x1.2c0002p+5f), 0x1.7ca610p+124f) && near(exp10f(-0x1.2c0002p+5f), 0x1.5856aap-125f));
	/* The m1 forms (P11.03f): each side of the series' end (|x ln b| = 1/8), the exp2 core's 2^30 and the hand-off above */
	QUIET(near(expm1f(0x1.fffffep-4f), 0x1.10b022p-3f) && near(expm1f(0x1p-3f), 0x1.10b022p-3f) && near(expm1f(-0x1.fffffep-4f), -0x1.e14aecp-4f) && near(expm1f(-0x1p-3f), -0x1.e14aeep-4f));
	QUIET(near(expm1f(0x1.ep+4f), 0x1.370470p+43f) && near(expm1f(0x1.5bfffep+6f), 0x1.6daf5ap+125f) && near(expm1f(0x1.5cp+6f), 0x1.6db012p+125f));
	QUIET(near(exp2m1f(0x1.6ffffep-3f), 0x1.0fa4c8p-3f) && near(exp2m1f(0x1.7p-3f), 0x1.0fa4c8p-3f) && near(exp2m1f(-0x1.7p-3f), -0x1.dfaa50p-4f) && near(exp2m1f(0x1.fp+4f), 0x1p+31f));
	QUIET(near(exp2m1f(0x1.f7fffep+6f), 0x1.ffff4ep+125f) && near(exp2m1f(-0x1.ap+4f), -1.0f));
	QUIET(near(exp10m1f(0x1.bbfffep-5f), 0x1.103852p-3f) && near(exp10m1f(0x1.bcp-5f), 0x1.103854p-3f) && near(exp10m1f(-0x1.bcp-5f), -0x1.e09046p-4f) && near(exp10m1f(0x1.2p+3f), 0x1.dcd650p+29f));
	QUIET(near(exp10m1f(0x1.2bfffep+5f), 0x1.7ca45ap+124f) && near(exp10m1f(0x1.2cp+5f), 0x1.7ca534p+124f) && near(exp10m1f(-0x1p+3f), -1.0f));
	/* The p1 forms (P11.03g): each side of the series' end (|x| = 2^-5), of 2^24 (where 1 + x stops being formed) and near -1 */
	QUIET(near(log1pf(0x1.fffffep-6f), 0x1.f829aep-6f) && near(log1pf(0x1p-5f), 0x1.f829b0p-6f) && near(log1pf(-0x1.fffffep-6f), -0x1.0415d8p-5f) && near(log1pf(-0x1p-5f), -0x1.0415d8p-5f));
	QUIET(near(log1pf(0x1.fffffep+22f), 0x1.fe2806p+3f) && near(log1pf(0x1p+23f), 0x1.fe2806p+3f) && near(log1pf(0x1p+24f), 0x1.0a2b24p+4f) && near(log1pf(-0x1.fffffep-1f), -0x1.0a2b24p+4f));
	QUIET(near(log2p1f(0x1.fffffep-6f), 0x1.6bad36p-5f) && near(log2p1f(0x1p-5f), 0x1.6bad38p-5f) && near(log2p1f(-0x1.fffffep-6f), -0x1.77394cp-5f) && near(log2p1f(-0x1p-5f), -0x1.77394cp-5f));
	QUIET(near(log2p1f(0x1.fffffep+22f), 23.0f) && near(log2p1f(0x1p+23f), 23.0f) && near(log2p1f(0x1p+24f), 24.0f) && near(log2p1f(-0x1.fffffep-1f), -24.0f));
	QUIET(near(log10p1f(0x1.fffffep-6f), 0x1.b5e908p-7f) && near(log10p1f(0x1p-5f), 0x1.b5e908p-7f) && near(log10p1f(-0x1.fffffep-6f), -0x1.c3d082p-7f) && near(log10p1f(-0x1p-5f), -0x1.c3d084p-7f));
	QUIET(near(log10p1f(0x1.fffffep+22f), 0x1.bb1dbcp+2f) && near(log10p1f(0x1p+23f), 0x1.bb1dbcp+2f) && near(log10p1f(0x1p+24f), 0x1.ce61d0p+2f) && near(log10p1f(-0x1.fffffep-1f), -0x1.ce61d0p+2f));
	/* cbrtf (P11.03g): each side of the powers 2^(3q + j) where j changes, and subnormals */
	QUIET(near(cbrtf(0x1.fffffep-1f), 1.0f) && near(cbrtf(2.0f), 0x1.428a30p+0f) && near(cbrtf(0x1.fffffep+0f), 0x1.428a30p+0f) && near(cbrtf(4.0f), 0x1.965feap+0f));
	QUIET(near(cbrtf(0x1.fffffep+2f), 2.0f) && near(cbrtf(8.0f), 2.0f) && near(cbrtf(0x1.8p-148f), 0x1.d12ed0p-50f) && near(cbrtf(0x1.fffffcp-127f), 0x1.fffffep-43f) && near(cbrtf(-0x1p-149f), -0x1.428a30p-50f));
}

int main(void)
{
	check_exponentials();
	check_logarithms();
	check_powers();
	check_minus_one();
	check_plus_one();
	check_cube_root();
	check_fixed_edges();

	if (failures == 0)
		puts("pass: libm_thresholds");
	return failures == 0 ? 0 : 1;
}
