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

#define _GNU_SOURCE

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

/* Within bound ulps of a double reference (near() allows 4) */
static bool within_ulps(float x, double want, double bound)
{
	int e;

	frexp(want, &e);
	if (e < -125)
		e = -125;
	return fabs((double)x - want) < bound * ldexp(1.0, e - 24);
}

/* Below 1 ulp of a double reference: the fast contract's bound */
static bool within_ulp(float x, double want)
{
	return within_ulps(x, want, 1.0);
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

static void check_trigonometric(void)
{
	/* C23 F.10.1.6, .5, .7 and the pi forms: NaN quietly, +-infinity invalid, +-0 kept by sin and tan, cos(+-0) = 1 */
	QUIET(isnan(sinf(NAN)) && isnan(cosf(NAN)) && isnan(tanf(NAN)) && isnan(sinpif(NAN)) && isnan(cospif(NAN)) && isnan(tanpif(NAN)));
	RAISES(isnan(sinf(INFINITY)) && isnan(cosf(-INFINITY)) && isnan(tanf(INFINITY)) && isnan(sinpif(-INFINITY)) && isnan(cospif(INFINITY)) && isnan(tanpif(INFINITY)), FE_INVALID);
	QUIET(is_positive_zero(sinf(0.0f)) && is_negative_zero(sinf(-0.0f)) && is_one(cosf(0.0f)) && is_one(cosf(-0.0f)) && is_negative_zero(tanf(-0.0f)));
	QUIET(is_positive_zero(sinpif(0.0f)) && is_negative_zero(sinpif(-0.0f)) && is_one(cospif(-0.0f)) && is_negative_zero(tanpif(-0.0f)));
	/* The pi forms' exact cases: the multiples of 1/2, and every float from 2^23 up (an integer) */
	QUIET(is_positive_zero(sinpif(1.0f)) && is_negative_zero(sinpif(-1.0f)) && sinpif(0.5f) == 1.0f && sinpif(-0.5f) == -1.0f && sinpif(2.5f) == 1.0f);
	QUIET(is_positive_zero(sinpif(0x1p23f)) && is_negative_zero(sinpif(-0x1.000002p23f)) && is_positive_zero(cospif(0.5f)) && cospif(1.0f) == -1.0f);
	QUIET(cospif(0x1p24f) == 1.0f && cospif(0x1.000002p23f) == -1.0f && is_positive_zero(tanpif(2.0f)) && is_negative_zero(tanpif(1.0f)));
	RAISES(tanpif(0.5f) == INFINITY && tanpif(1.5f) == -INFINITY && tanpif(-0.5f) == -INFINITY, FE_DIVBYZERO);
	/* Each side of the reductions' edges (2^20 for the double cores' Cody-Waite, 2^-12 and 2^-6 for the float cores), pi itself, and beyond 2^24 */
	QUIET(near(sinf(0x1.fffffep+19f), 0x1.156658p-2f) && near(sinf(0x1p+20f), 0x1.526cccp-2f) && near(sinf(0x1.fffffep-7f), 0x1.fffaa8p-7f) && near(sinf(0x1p-6f), 0x1.fffaaap-7f));
	QUIET(near(sinf(0x1.fffffep-13f), 0x1.fffffep-13f) && near(sinf(0x1p-12f), 0x1p-12f) && near(sinf(0x1.921fb6p+1f), -0x1.777a5cp-24f) && near(sinf(0x1.921fb6p+0f), 1.0f) && near(sinf(0x1.000002p+24f), 0x1.c9b0c8p-1f));
	QUIET(near(cosf(0x1.fffffep+19f), 0x1.ecdaf4p-1f) && near(cosf(0x1p+20f), 0x1.e33adap-1f) && near(cosf(0x1.fffffep-7f), 0x1.fff000p-1f) && near(cosf(0x1p-6f), 0x1.fff000p-1f));
	QUIET(near(cosf(0x1.921fb6p+1f), -1.0f) && near(cosf(0x1.921fb6p+0f), -0x1.777a5cp-25f) && near(cosf(0x1.000002p+24f), 0x1.caf854p-2f));
	QUIET(near(tanf(0x1.fffffep+19f), 0x1.202cdcp-2f) && near(tanf(0x1p+20f), 0x1.6692e6p-2f) && near(tanf(0x1.fffffep-7f), 0x1.000554p-6f) && near(tanf(0x1p-6f), 0x1.000556p-6f));
	/* Each side of 64, where the float cores' Cody-Waite reduction hands over, and its closest float to a multiple of pi/2 */
	QUIET(near(sinf(0x1.fffffep+5f), 0x1.d70d74p-1f) && near(cosf(0x1.fffffep+5f), 0x1.9143f2p-2f) && near(sinf(0x1.000000p+6f), 0x1.d70da8p-1f) && near(cosf(0x1.000000p+6f), 0x1.914306p-2f) && near(sinf(0x1.2d97c8p+2f), -0x1.000000p+0f) && near(cosf(0x1.2d97c8p+2f), 0x1.99bc5cp-27f));
	QUIET(near(tanf(0x1.921fb6p+1f), 0x1.777a5cp-24f) && near(tanf(0x1.921fb6p+0f), -0x1.5d1494p+24f) && near(tanf(0x1.000002p+24f), 0x1.fe929cp+0f));
	QUIET(near(sinpif(0.25f), 0x1.6a09e6p-1f) && near(sinpif(0x1.99999ap-4f), 0x1.3c6ef4p-2f) && near(sinpif(-0x1.333334p-2f), -0x1.9e377ap-1f) && near(sinpif(1.75f), -0x1.6a09e6p-1f));
	QUIET(near(sinpif(1000.125f), 0x1.87de2ap-2f) && near(sinpif(0x1p-100f), 0x1.921fb6p-99f) && near(cospif(0x1.99999ap-4f), 0x1.e6f0e2p-1f) && near(cospif(-0x1.333334p-2f), 0x1.2cf230p-1f));
	QUIET(near(cospif(1000.125f), 0x1.d906bcp-1f) && near(tanpif(0.25f), 1.0f) && near(tanpif(0x1.99999ap-4f), 0x1.4cb7c0p-2f) && near(tanpif(-0x1.333334p-2f), -0x1.605a92p+0f));
	QUIET(near(tanpif(1.75f), -1.0f) && near(tanpif(1000.125f), 0x1.a8279ap-2f) && near(tanpif(0x1p-100f), 0x1.921fb6p-99f));
	/*
	 * The fixed-point paths of the cores without an FPU, within 1 ulp: the pi forms each side of 2^-90 (pi x) and 2^18 (32x an
	 * integer), cospif of 2^-15 (1), next to integers and half-integers (the small-angle path where the kernel declines), and
	 * sinf and cosf next to multiples of pi and pi/2 (the float-float core)
	 */
	QUIET(within_ulp(sinpif(0x1.fffffep-91f), 0x1.921fb3b2231c4p-89) && within_ulp(sinpif(0x1.000002p-90f), 0x1.921fb868823c1p-89) && within_ulp(tanpif(0x1.fffffep-91f), 0x1.921fb3b2231c4p-89) && within_ulp(tanpif(0x1.000002p-90f), 0x1.921fb868823c1p-89));
	QUIET(within_ulp(sinpif(0x1.fffffep+17f), -0x1.91f65f10dd814p-5) && within_ulp(sinpif(0x1.000002p+18f), 0x1.917a6bc29b42cp-4) && within_ulp(tanpif(0x1.fffffep+17f), -0x1.927278a3b1162p-5) && within_ulp(tanpif(0x1.000002p+18f), 0x1.936bb8c5b2da2p-4));
	QUIET(within_ulp(cospif(0x1.fffffep-16f), 0x1.ffffffd88586cp-1) && within_ulp(cospif(0x1.000002p-15f), 0x1.ffffffd88585dp-1) && within_ulp(cospif(0x1.fffffep+17f), 0x1.ff621e3796d7ep-1) && within_ulp(cospif(0x1.000002p+18f), 0x1.fd88da3d12526p-1));
	QUIET(within_ulp(sinpif(0x1.000008p+0f), -0x1.921fb54442297p-20) && within_ulp(cospif(0x1.000008p+0f), -0x1.fffffffffd886p-1) && within_ulp(tanpif(0x1.000008p+0f), 0x1.921fb544441c4p-20) && within_ulp(sinpif(0x1.fffff8p+1f), -0x1.921fb544401bcp-19));
	QUIET(within_ulp(cospif(0x1.fffff8p+1f), 0x1.fffffffff6216p-1) && within_ulp(tanpif(0x1.fffff8p+1f), -0x1.921fb54448039p-19) && within_ulp(sinpif(0x1.000004p-1f), 0x1.ffffffffffd88p-1) && within_ulp(cospif(0x1.000004p-1f), -0x1.921fb54443134p-22));
	QUIET(within_ulp(tanpif(0x1.000004p-1f), -0x1.45f306dc9c39cp+21) && within_ulp(sinpif(0x1.7ffffcp+0f), -0x1.ffffffffff621p-1) && within_ulp(cospif(0x1.7ffffcp+0f), -0x1.921fb544426a4p-21) && within_ulp(tanpif(0x1.7ffffcp+0f), 0x1.45f306dc9c5f6p+20));
	QUIET(within_ulp(sinf(0x1.921fb6p+1f), -0x1.777a5cf72cec6p-24) && within_ulp(cosf(0x1.921fb6p+1f), -0x1.fffffffffffdep-1) && within_ulp(sinf(0x1.921fb6p+2f), 0x1.777a5cf72ceadp-23) && within_ulp(cosf(0x1.921fb6p+2f), 0x1.fffffffffff76p-1));
	QUIET(within_ulp(sinf(0x1.2d97c8p+2f), -0x1.fffffffffffffp-1) && within_ulp(cosf(0x1.2d97c8p+2f), 0x1.99bc5b961b1adp-27) && within_ulp(sinf(0x1.f6a7a2p+3f), 0x1.2aa70bcb07d3ap-22) && within_ulp(cosf(0x1.f6a7a2p+3f), -0x1.ffffffffffea4p-1));
}

static void check_inverse_trigonometric(void)
{
	/* C23 F.10.1.1-4 and the pi forms: NaN quietly, |x| > 1 invalid, +-0 kept, the exact values */
	QUIET(isnan(asinf(NAN)) && isnan(acosf(NAN)) && isnan(atanf(NAN)) && isnan(atan2f(NAN, 1.0f)) && isnan(asinpif(NAN)) && isnan(atanpif(NAN)));
	RAISES(isnan(asinf(1.5f)) && isnan(acosf(-0x1.000002p0f)) && isnan(asinpif(INFINITY)) && isnan(acospif(-2.0f)), FE_INVALID);
	QUIET(is_positive_zero(asinf(0.0f)) && is_negative_zero(asinf(-0.0f)) && is_negative_zero(atanf(-0.0f)) && is_positive_zero(acosf(1.0f)) && is_negative_zero(atanpif(-0.0f)));
	QUIET(asinpif(1.0f) == 0.5f && asinpif(-1.0f) == -0.5f && is_positive_zero(acospif(1.0f)) && acospif(-1.0f) == 1.0f && acospif(0.0f) == 0.5f);
	QUIET(atanpif(INFINITY) == 0.5f && atanpif(-INFINITY) == -0.5f && atanpif(1.0f) == 0.25f && near(atanf(INFINITY), 0x1.921fb6p0f) && near(acosf(-1.0f), 0x1.921fb6p1f));
	/* atan2: the signed zeros and infinities, and atan2pi's exact values */
	QUIET(is_negative_zero(atan2f(-0.0f, 1.0f)) && near(atan2f(0.0f, -1.0f), 0x1.921fb6p1f) && near(atan2f(-0.0f, -0.0f), -0x1.921fb6p1f) && near(atan2f(1.0f, 0.0f), 0x1.921fb6p0f));
	QUIET(near(atan2f(INFINITY, -INFINITY), 0x1.2d97c8p1f) && near(atan2f(-INFINITY, INFINITY), -0x1.921fb6p-1f) && is_negative_zero(atan2f(-1.0f, INFINITY)) && near(atan2f(1.0f, -INFINITY), 0x1.921fb6p1f));
	QUIET(atan2pif(0.0f, -1.0f) == 1.0f && atan2pif(-1.0f, 0.0f) == -0.5f && atan2pif(INFINITY, -INFINITY) == 0.75f && atan2pif(-3.0f, 3.0f) == -0.25f && atan2pif(2.0f, -2.0f) == 0.75f);
	/* Each side of 1/2 (asin and acos change form) and of 1 (atan inverts), near +-1, and small arguments */
	QUIET(near(asinf(0x1.fffffep-2f), 0x1.0c1522p-1f) && near(asinf(0.5f), 0x1.0c1524p-1f) && near(asinf(0x1.000002p-1f), 0x1.0c1526p-1f) && near(asinf(-0x1.000002p-1f), -0x1.0c1526p-1f));
	QUIET(near(asinf(0x1.fffffep-1f), 0x1.920914p+0f) && near(asinf(-0x1.fffffep-1f), -0x1.920914p+0f));
	QUIET(near(acosf(0x1.fffffep-2f), 0x1.0c1524p+0f) && near(acosf(0x1.000002p-1f), 0x1.0c1522p+0f) && near(acosf(-0x1.000002p-1f), 0x1.0c1524p+1f) && near(acosf(0x1.fffffep-1f), 0x1.6a09e6p-12f));
	QUIET(near(acosf(-0x1.fffffep-1f), 0x1.921464p+1f) && near(acosf(-0.25f), 0x1.d2cf5cp+0f));
	QUIET(near(atanf(0x1.fffffep-1f), 0x1.921fb4p-1f) && near(atanf(0x1.000002p+0f), 0x1.921fb8p-1f) && near(atanf(-0x1.000002p+0f), -0x1.921fb8p-1f) && near(atanf(0x1p+24f), 0x1.921fb4p+0f) && near(atanf(0x1p-11f), 0x1.fffffep-12f));
	QUIET(near(asinpif(0x1.000002p-1f), 0x1.555558p-3f) && near(asinpif(-0x1.fffffep-1f), -0x1.ffe330p-2f) && near(asinpif(0.125f), 0x1.46cddep-5f));
	QUIET(near(acospif(0x1.000002p-1f), 0x1.555554p-2f) && near(acospif(-0x1.000002p-1f), 0x1.555556p-1f) && near(acospif(0x1.fffffep-1f), 0x1.ccf642p-14f));
	QUIET(near(atanpif(0x1.000002p+0f), 0x1.000002p-2f) && near(atanpif(-100.0f), -0x1.fcbd98p-2f) && near(atanpif(0.0625f), 0x1.4586a2p-6f));
	/* atan2 in each quadrant, both sides of |y| = |x|, and a far smaller y beside a negative x */
	QUIET(near(atan2f(1.0f, 2.0f), 0x1.dac670p-2f) && near(atan2f(2.0f, 1.0f), 0x1.1b6e1ap+0f) && near(atan2f(1.0f, -2.0f), 0x1.56c6e8p+1f) && near(atan2f(-1.0f, -2.0f), -0x1.56c6e8p+1f));
	QUIET(near(atan2f(3.0f, -0.5f), 0x1.bc66e4p+0f) && near(atan2f(0x1.79ca10p-67f, -1.0f), 0x1.921fb6p+1f));
	QUIET(near(atan2pif(1.0f, 2.0f), 0x1.2e4052p-3f) && near(atan2pif(2.0f, 1.0f), 0x1.68dfd8p-2f) && near(atan2pif(1.0f, -2.0f), 0x1.b46fecp-1f) && near(atan2pif(-1.0f, -2.0f), -0x1.b46fecp-1f));
	QUIET(near(atan2pif(3.0f, -0.5f), 0x1.1aea40p-1f) && near(atan2pif(0x1.79ca10p-67f, -1.0f), 1.0f));
	/*
	 * regression: the float atan kernel's j = a 8 + 1/2 rounded up just below 1/16, and a - 1/8 was then inexact (1.27
	 * ulp); a subnormal y lost the quotient's remainder (1.43 ulp) and a subnormal pi form rounded twice (1.65, 0.999);
	 * accuracy only: a subnormal result may raise underflow
	 */
	CHECK(within_ulp(atanf(0x1.fffffep-5f), 0x1.ff55b974cde0ap-5) && within_ulp(atanpif(0x1.29a8c4p-124f), 0x1.7afdd40083a1ap-126));
	CHECK(within_ulp(atan2f(-0x1.d0f908p-128f, 0x1.cf06bep-126f), -0x1.f7bdd378a59ccp-3) && within_ulp(atan2f(-0x1.70ca9p-128f, 0x1.707228p-124f), -0x1.ffd01ada0df0bp-5));
	/* regression: a quotient near 2^-125 had its remainder rounded to half an ulp of it (0.67 ulp; complex_float's 0.52) */
	CHECK(within_ulps(atan2f(-0x1.f59efp-36f, 0x1.6f5952p+89f), -0x1.5d9276a8b0b88p-125, 0.52) && within_ulps(atan2f(0x1.821528p+3f, 0x1.f6b5dep+126f), 0x1.893791320e1ffp-124, 0.52));
	CHECK(within_ulp(atan2pif(-0x1.18fe76p-125f, 0x1.f9920ap-2f), -0x1.6a52954a74b4bp-126) && within_ulp(atan2pif(-0x1.0ec7f6p-126f, 0x1.68b2d6p-119f), -0x1.e962164514f9dp-10));
	/*
	 * The cores without an FPU (P11.04f1): each side of the kernel's series and pieces at 1/8 and of the 2^26 and 2^27 ends,
	 * the sweeps' worst points, and tiny ratios down to zero
	 */
	QUIET(within_ulp(atanf(0x1.fffffep-4f), 0x1.fd5ba7b2a374ep-4) && within_ulp(atanf(0x1.000002p-3f), 0x1.fd5bad9b01fabp-4) && within_ulp(atanf(0x1.00418cp-3f), 0x1.fddcbcd9c9e98p-4));
	QUIET(within_ulp(atanf(0x1.fffffep+25f), 0x1.921fb50442d14p+0) && within_ulp(atanf(0x1.000002p+26f), 0x1.921fb50442d20p+0) && within_ulp(atanpif(0x1.0087a2p-3f), 0x1.44ee7cebb95b4p-5));
	QUIET(within_ulp(atanpif(0x1.fffffep+26f), 0x1.ffffffd7419f0p-2) && within_ulp(atan2f(-0x1.11ba3cp+72f, 0x1.1118eap+75f), -0x1.fe8570dcce5fcp-4));
	CHECK(within_ulp(atanpif(0x1p-149f), 0x1.45f306dc9c883p-151) && within_ulp(atan2pif(-0x1.17ae5p-113f, 0x1.721ed8p-111f), -0x1.e6de531990acfp-5) && is_positive_zero(atan2f(0x1p-149f, 0x1p+127f)));
	/*
	 * The cores without an FPU (P11.04f2): each side of asinf's 2^-12, of acosf's and acospif's 2^-26 and of the series'
	 * u = 0 at 2^-16, a subnormal asinpif, and the sweeps' worst points
	 */
	QUIET(within_ulp(asinf(0x1.fffffep-13f), 0x1.fffffe5555548p-13) && within_ulp(asinf(0x1.000002p-12f), 0x1.0000022aaaabcp-12) && within_ulp(asinf(0x1.000002p-16f), 0x1.000002002aaabp-16));
	QUIET(within_ulp(acosf(0x1.fffffep-27f), 0x1.921fb50442d1cp+0) && within_ulp(acosf(-0x1.000002p-26f), 0x1.921fb58442d20p+0) && within_ulp(acospif(-0x1.000002p-26f), 0x1.00000028be613p-1));
	QUIET(within_ulp(asinf(0x1.2a5388p-1f), 0x1.3e77e0fbbd716p-1) && within_ulp(acosf(0x1.2acdcap-1f), 0x1.e5310b046f730p-1) && within_ulp(asinpif(0x1.011c9p-1f), 0x1.56f800f3bb1bbp-3));
	QUIET(within_ulp(acospif(0x1.d96ec8p-1f), 0x1.fd49ed079f8d6p-4));
	CHECK(within_ulp(asinpif(0x1p-149f), 0x1.45f306dc9c883p-151));
	/* regression: the square root's remainder taken as positive where the last Newton step rounded r up (216-322 ulp, P11.04f2) */
	QUIET(within_ulp(asinf(0x1.acde6ap-1f), 0x1.fc625c7f784d4p-1) && within_ulp(acosf(0x1.bcaffep-1f), 0x1.0981a42ab7d50p-1) && within_ulp(acosf(-0x1.bcaffep-1f), 0x1.4fbf4c3994dc4p+1));
	QUIET(within_ulp(asinpif(0x1.658ca6p-1f), 0x1.f7f73deff3a8cp-3) && within_ulp(acospif(0x1.bcaf8p-1f), 0x1.520ee644c50d6p-3));
	/* regression: an exact ratio of 1/8 gave a quotient a unit below 1/2 and the piece below the first (5411 ulp, P11.04f1) */
	QUIET(within_ulp(atan2f(-0x1.2d625cp-59f, -0x1.2d625cp-62f), -0x1.b1f56fdeef00fp+0) && within_ulp(atan2f(0x1.2d625cp-62f, 0x1.2d625cp-59f), 0x1.fd5ba9aac2f6ep-4));
	QUIET(within_ulp(atan2pif(-0x1.2d625cp-59f, -0x1.2d625cp-62f), -0x1.1444475077767p-1));
}

static void check_hyperbolic(void)
{
	/* C23 F.10.2: the zeros with their signs, the infinities, NaN quietly, the poles and the domain errors */
	QUIET(is_positive_zero(sinhf(0.0f)) && is_negative_zero(sinhf(-0.0f)) && sinhf(INFINITY) == INFINITY && sinhf(-INFINITY) == -INFINITY && isnan(sinhf(NAN)));
	QUIET(coshf(0.0f) == 1.0f && coshf(-0.0f) == 1.0f && coshf(-INFINITY) == INFINITY && isnan(coshf(NAN)));
	QUIET(is_negative_zero(tanhf(-0.0f)) && tanhf(INFINITY) == 1.0f && tanhf(-INFINITY) == -1.0f && tanhf(9.2f) == 1.0f && tanhf(-20.0f) == -1.0f && isnan(tanhf(NAN)));
	QUIET(is_negative_zero(asinhf(-0.0f)) && asinhf(-INFINITY) == -INFINITY && isnan(asinhf(NAN)));
	QUIET(is_positive_zero(acoshf(1.0f)) && acoshf(INFINITY) == INFINITY && isnan(acoshf(NAN)) && is_negative_zero(atanhf(-0.0f)) && isnan(atanhf(NAN)));
	RAISES(isnan(acoshf(0.5f)) && isnan(acoshf(-INFINITY)) && isnan(atanhf(1.5f)) && isnan(atanhf(-INFINITY)), FE_INVALID);
	RAISES(atanhf(1.0f) == INFINITY && atanhf(-1.0f) == -INFINITY, FE_DIVBYZERO);
	RAISES(sinhf(89.5f) == INFINITY && sinhf(-90.0f) == -INFINITY && coshf(89.5f) == INFINITY && coshf(-200.0f) == INFINITY, FE_OVERFLOW);
	/*
	 * Each side of every edge (the tiny paths at 2^-30 and 2^-12; the series and the exponential at 1/2 and 1 for sinhf,
	 * 1/4 and 9 for coshf; 1/16, 1 and 9.1 for tanhf; 1/4 and 2^12 for asinhf, 2 and 2^12 for acoshf, 1/8, 1/2 and 1 for
	 * atanhf), within the fast contract's 1.5 ulp
	 */
	QUIET(within_ulps(sinhf(0x1.fffffep-13f), 0x1.fffffe5555546p-13, 1.5) && within_ulps(sinhf(0x1.000002p-12f), 0x1.0000022aaaabbp-12, 1.5) && within_ulps(sinhf(0x1.fffffep-31f), 0x1.fffffep-31, 1.5));
	QUIET(within_ulps(sinhf(0x1.000002p-30f), 0x1.000002p-30, 1.5) && within_ulps(sinhf(0x1.fffffep-2f), 0x1.0accffddb7a13p-1, 1.5) && within_ulps(sinhf(0x1.000002p-1f), 0x1.0acd033fbbeabp-1, 1.5));
	QUIET(within_ulps(sinhf(0x1.fffffep-1f), 0x1.2cd9fab9e443ap+0, 1.5) && within_ulps(sinhf(0x1.000002p+0f), 0x1.2cd9ff5afa44cp+0, 1.5) && within_ulps(sinhf(0x1.1ffffep+3f), 0x1.fa71379bf4dfdp+11, 1.5));
	QUIET(within_ulps(sinhf(0x1.200002p+3f), 0x1.fa7176ea1fe89p+11, 1.5) && within_ulps(sinhf(0x1.659998p+6f), 0x1.f7e08458f02b2p+127, 1.5) && within_ulps(sinhf(0x1.65999cp+6f), 0x1.f7e27c3a7074bp+127, 1.5));
	QUIET(within_ulps(sinhf(-0x1.7ffffep+1f), -0x1.40926967eb7efp+3, 1.5));
	QUIET(within_ulps(coshf(0x1.fffffep-13f), 0x1.0000007fffff1p+0, 1.5) && within_ulps(coshf(0x1.000002p-12f), 0x1.0000008000021p+0, 1.5) && within_ulps(coshf(0x1.fffffep-3f), 0x1.080ab04c7b479p+0, 1.5));
	QUIET(within_ulps(coshf(0x1.000002p-2f), 0x1.080ab07cfbae3p+0, 1.5) && within_ulps(coshf(0x1.1ffffep+3f), 0x1.fa71389ec4125p+11, 1.5) && within_ulps(coshf(0x1.200002p+3f), 0x1.fa7177eceefabp+11, 1.5));
	QUIET(within_ulps(coshf(0x1.659998p+6f), 0x1.f7e08458f02b2p+127, 1.5) && within_ulps(coshf(0x1.65999cp+6f), 0x1.f7e27c3a7074bp+127, 1.5) && within_ulps(coshf(-0x1.fffffep-1f), 0x1.8b0753f0c5598p+0, 1.5));
	QUIET(within_ulps(tanhf(0x1.fffffep-13f), 0x1.fffffd555557ap-13, 1.5) && within_ulps(tanhf(0x1.000002p-12f), 0x1.000001aaaaa8dp-12, 1.5) && within_ulps(tanhf(0x1.fffffep-31f), 0x1.fffffep-31, 1.5));
	QUIET(within_ulps(tanhf(0x1.000002p-30f), 0x1.000002p-30, 1.5) && within_ulps(tanhf(0x1.fffffep-5f), 0x1.ff55978001b8ep-5, 1.5) && within_ulps(tanhf(0x1.000002p-4f), 0x1.ff559d7a05b69p-5, 1.5));
	QUIET(within_ulps(tanhf(0x1.fffffep-1f), 0x1.85efaa7a48582p-1, 1.5) && within_ulps(tanhf(0x1.000002p+0f), 0x1.85efacff5cf7bp-1, 1.5) && within_ulps(tanhf(0x1.233332p+3f), 0x1.ffffff29c7959p-1, 1.5));
	QUIET(within_ulps(tanhf(0x1.233336p+3f), 0x1.ffffff29c7cb2p-1, 1.5) && within_ulps(tanhf(-0x1.fffffep-2f), -0x1.d9353be2bf67ep-2, 1.5));
	QUIET(within_ulps(asinhf(0x1.fffffep-13f), 0x1.fffffdaaaaabdp-13, 1.5) && within_ulps(asinhf(0x1.000002p-12f), 0x1.000001d555547p-12, 1.5) && within_ulps(asinhf(0x1.fffffep-31f), 0x1.fffffep-31, 1.5));
	QUIET(within_ulps(asinhf(0x1.000002p-30f), 0x1.000002p-30, 1.5) && within_ulps(asinhf(0x1.fffffep-3f), 0x1.facfb048e7dedp-3, 1.5) && within_ulps(asinhf(0x1.000002p-2f), 0x1.facfb61b0b6c5p-3, 1.5));
	QUIET(within_ulps(asinhf(0x1.fffffep+11f), 0x1.205966dab4f12p+3, 1.5) && within_ulps(asinhf(0x1.000002p+12f), 0x1.2059673ab4f0cp+3, 1.5) && within_ulps(asinhf(-0x1.bffffep+2f), -0x1.52728c00e52efp+1, 1.5));
	QUIET(within_ulps(acoshf(0x1.000002p+0f), 0x1.ffffffaaaaaadp-12, 1.5) && within_ulps(acoshf(0x1.fffffep+11f), 0x1.205966cab4f1p+3, 1.5) && within_ulps(acoshf(0x1.000002p+12f), 0x1.2059672ab4f1p+3, 1.5));
	QUIET(within_ulps(acoshf(0x1.fffffep+0f), 0x1.512425f1e5ce3p+0, 1.5) && within_ulps(acoshf(0x1.000002p+1f), 0x1.51242968b528ep+0, 1.5));
	QUIET(within_ulps(atanhf(0x1.fffffep-1f), 0x1.1542456b37d43p+3, 1.5) && within_ulps(atanhf(-0x1.fffffep-1f), -0x1.1542456b37d43p+3, 1.5) && within_ulps(atanhf(0x1.fffffep-13f), 0x1.fffffeaaaaa91p-13, 1.5));
	QUIET(within_ulps(atanhf(0x1.000002p-12f), 0x1.0000025555579p-12, 1.5) && within_ulps(atanhf(0x1.fffffep-31f), 0x1.fffffep-31, 1.5) && within_ulps(atanhf(0x1.000002p-30f), 0x1.000002p-30, 1.5));
	QUIET(within_ulps(atanhf(0x1.fffffep-4f), 0x1.015890c5daae7p-3, 1.5) && within_ulps(atanhf(0x1.000002p-3f), 0x1.015893d20b719p-3, 1.5) && within_ulps(atanhf(0x1.fffffep-2f), 0x1.193ea6557adbcp-1, 1.5));
	QUIET(within_ulps(atanhf(0x1.000002p-1f), 0x1.193eaa557add2p-1, 1.5));
	/*
	 * The cores without an FPU (P11.04g1): each side of the series' end at 1/2, of the second exponential's at 11 and of tanh's
	 * end at 9.1, the largest finite sinh and cosh and the first overflow, and the sweeps' worst points
	 */
	QUIET(within_ulps(sinhf(0x1.fffffep-2f), 0x1.0accffddb7a13p-1, 1.5) && within_ulps(sinhf(0x1.000002p-1f), 0x1.0acd033fbbeabp-1, 1.5) && within_ulps(sinhf(0x1.5ffffep+3f), 0x1.d3c42b5070ee4p+14, 1.5));
	QUIET(within_ulps(sinhf(0x1.600002p+3f), 0x1.d3c465c8fa006p+14, 1.5) && within_ulps(sinhf(-0x1.65a9f8p+6f), -0x1.ffffd83e8e728p+127, 1.5) && within_ulps(sinhf(0x1.aae622p-1f), 0x1.de1c470438762p-1, 1.5));
	QUIET(within_ulps(coshf(0x1.000002p-12f), 0x1.0000008000021p+0, 1.5) && within_ulps(coshf(0x1.5ffffep+3f), 0x1.d3c42b54d1c39p+14, 1.5) && within_ulps(coshf(0x1.600002p+3f), 0x1.d3c465cd5ad52p+14, 1.5));
	QUIET(within_ulps(coshf(0x1.65a9f8p+6f), 0x1.ffffd83e8e728p+127, 1.5) && within_ulps(coshf(0x1.6215d2p+3f), 0x1.f3435f0442a2ap+14, 1.5) && within_ulps(tanhf(0x1.fffffep-2f), 0x1.d9353be2bf67ep-2, 1.5));
	QUIET(within_ulps(tanhf(0x1.000002p-1f), 0x1.d935409abb3bbp-2, 1.5) && within_ulps(tanhf(0x1.233332p+3f), 0x1.ffffff29c7959p-1, 1.5) && within_ulps(tanhf(-0x1.18f70cp-1f), -0x1.ff948efd0fd25p-2, 1.5));
	RAISES(sinhf(0x1.65a9fap+6f) == INFINITY && coshf(-0x1.65a9fap+6f) == INFINITY, FE_OVERFLOW);
	/*
	 * The cores without an FPU (P11.04g2): each side of asinh's series at 1/2, of acosh's at 1.25, of atanh's at 1/4 and of
	 * the logarithm's 2 |x| from 2^12, the floats next to 1 and FLT_MAX, and the sweeps' worst points
	 */
	QUIET(within_ulps(asinhf(0x1.000002p-1f), 0x1.ecc2ce803618ap-2, 1.5) && within_ulps(asinhf(0x1.fffffep+11f), 0x1.205966dab4f12p+3, 1.5) && within_ulps(asinhf(-0x1p+12f), -0x1.205966fab4f12p+3, 1.5));
	QUIET(within_ulps(asinhf(0x1.0aa736p-1f), 0x1.ffbcf70d4df2fp-2, 1.5) && within_ulps(asinhf(0x1.fffffep+127f), 0x1.65a9f84b82e63p+6, 1.5) && within_ulps(acoshf(0x1.000002p+0f), 0x1.ffffffaaaaaadp-12, 1.5));
	QUIET(within_ulps(acoshf(0x1.4p+0f), 0x1.62e42fefa39efp-1, 1.5) && within_ulps(acoshf(0x1.400002p+0f), 0x1.62e43544f8e87p-1, 1.5) && within_ulps(acoshf(0x1.fffffep+11f), 0x1.205966cab4f10p+3, 1.5));
	QUIET(within_ulps(acoshf(0x1.00536ep+12f), 0x1.2063d2f8105e8p+3, 1.5) && within_ulps(acoshf(0x1.fffffep+127f), 0x1.65a9f84b82e63p+6, 1.5) && within_ulps(atanhf(0x1.fffffep-3f), 0x1.058aee9700342p-2, 1.5));
	QUIET(within_ulps(atanhf(0x1p-2f), 0x1.058aefa811452p-2, 1.5) && within_ulps(atanhf(-0x1.fffffep-1f), -0x1.1542456b37d43p+3, 1.5) && within_ulps(atanhf(0x1.d89f58p-2f), 0x1.ff4176fb01c01p-2, 1.5));
	/* regression: asinhf below 2^-12 returned x + x, twice its argument, in its first build (P11.04g2) */
	QUIET(asinhf(0x1p-13f) == 0x1p-13f && asinhf(-0x1.fffffep-13f) == -0x1.fffffep-13f);
}

static void check_error_functions(void)
{
	/* C23 F.10.5: the zeros with their signs, the infinities, NaN quietly, 1 and 2 beyond the ends, underflow */
	QUIET(is_positive_zero(erff(0.0f)) && is_negative_zero(erff(-0.0f)) && erff(INFINITY) == 1.0f && erff(-INFINITY) == -1.0f && erff(4.0f) == 1.0f && erff(-10.0f) == -1.0f && isnan(erff(NAN)));
	QUIET(erfcf(0.0f) == 1.0f && erfcf(-0.0f) == 1.0f && is_positive_zero(erfcf(INFINITY)) && erfcf(-INFINITY) == 2.0f && erfcf(-4.0f) == 2.0f && isnan(erfcf(NAN)));
	RAISES(is_positive_zero(erfcf(10.5f)) && is_positive_zero(erfcf(100.0f)), FE_UNDERFLOW);
	/*
	 * Each side of every edge (erff's series at 2^-13 and 1/2, the pieces at 1, 1.5, 2, 2.5, 3 and the end at 3.92; erfcf's
	 * at 2^-26, +-1/2, the pieces, 3 and -3.9), within the fast contract's bounds (1.5 ulp for erff, 1 for erfcf)
	 */
	QUIET(within_ulps(erff(0x1.fffffep-14f), 0x1.20dd73cb39cdbp-13, 1.5) && within_ulps(erff(0x1.000002p-13f), 0x1.20dd772dd22bep-13, 1.5) && within_ulps(erff(0x1.fffffep-2f), 0x1.0a7ef4e096f7cp-1, 1.5));
	QUIET(within_ulps(erff(0x1.000002p-1f), 0x1.0a7ef7837ea69p-1, 1.5) && within_ulps(erff(0x1.fffffep-1f), 0x1.af76799f878f9p-1, 1.5) && within_ulps(erff(0x1.000002p+0f), 0x1.af767c1d2275fp-1, 1.5));
	QUIET(within_ulps(erff(0x1.7ffffep+0f), 0x1.eea554f76ecffp-1, 1.5) && within_ulps(erff(0x1.800002p+0f), 0x1.eea555eb00893p-1, 1.5) && within_ulps(erff(0x1.fffffep+0f), 0x1.fd9ae12d4fa3fp-1, 1.5));
	QUIET(within_ulps(erff(0x1.000002p+1f), 0x1.fd9ae16cccd0bp-1, 1.5) && within_ulps(erff(0x1.3ffffep+1f), 0x1.ffcaa8f053b22p-1, 1.5) && within_ulps(erff(0x1.400002p+1f), 0x1.ffcaa8f93fcadp-1, 1.5));
	QUIET(within_ulps(erff(0x1.7ffffep+1f), 0x1.fffd1ac3ca5d6p-1, 1.5) && within_ulps(erff(0x1.800002p+1f), 0x1.fffd1ac45c61dp-1, 1.5) && within_ulps(erff(0x1.f5c28ep+1f), 0x1.ffffff01a3185p-1, 1.5));
	QUIET(within_ulps(erff(0x1.f5c292p+1f), 0x1.ffffff01a3588p-1, 1.5) && within_ulps(erff(-0x1.bffffep-1f), -0x1.917248450d998p-1, 1.5) && within_ulps(erff(-0x1.c00002p-1f), -0x1.91724a5e645d6p-1, 1.5));
	/*
	 * The cores without an FPU (P11.04e2): their last piece at 3.5, the series' u = 0 at 2^-16, subnormal arguments and
	 * results, and the sweep's worst point (0.5223 ulp)
	 */
	QUIET(within_ulps(erff(0x1.bffffep+1f), 0x1.ffffe710d2913p-1, 1.5) && within_ulps(erff(0x1.c00002p+1f), 0x1.ffffe710d83aap-1, 1.5) && within_ulps(erff(0x1.fffffep-17f), 0x1.20dd73e2ebf80p-16, 1.5));
	QUIET(within_ulps(erff(0x1.000002p-16f), 0x1.20dd774584570p-16, 1.5) && within_ulps(erff(0x1.7acf48p+0f), 0x1.ed5f850b67f38p-1, 1.5));
	CHECK(within_ulps(erff(0x1.fffffcp-127f), 0x1.20dd72c26eccdp-126, 1.5) && within_ulps(erff(0x1p-149f), 0x1.20dd750429b6dp-149, 1.5) && within_ulps(erff(-0x1.3p-140f), -0x1.5706faf4f1892p-140, 1.5));
	QUIET(within_ulps(erfcf(0x1.fffffep-27f), 0x1.ffffff6f91461p-1, 1) && within_ulps(erfcf(0x1.000002p-26f), 0x1.ffffff6f91446p-1, 1) && within_ulps(erfcf(0x1.fffffep-2f), 0x1.eb02163ed2107p-2, 1));
	QUIET(within_ulps(erfcf(0x1.000002p-1f), 0x1.eb0210f902b2fp-2, 1) && within_ulps(erfcf(-0x1.fffffep-2f), 0x1.853f7a704b7bep+0, 1) && within_ulps(erfcf(-0x1.000002p-1f), 0x1.853f7bc1bf534p+0, 1));
	QUIET(within_ulps(erfcf(0x1.fffffep-1f), 0x1.42261981e1c1dp-3, 1) && within_ulps(erfcf(0x1.000002p+0f), 0x1.42260f8b76284p-3, 1) && within_ulps(erfcf(0x1.fffffep+0f), 0x1.328f69582e052p-8, 1));
	QUIET(within_ulps(erfcf(0x1.000002p+1f), 0x1.328f499997a6ep-8, 1) && within_ulps(erfcf(0x1.7ffffep+1f), 0x1.729e1ad1530ffp-16, 1) && within_ulps(erfcf(0x1.800002p+1f), 0x1.729dd1cf18a16p-16, 1));
	QUIET(within_ulps(erfcf(0x1.bffffep+1f), 0x1.8ef2d6ecbe42cp-21, 1) && within_ulps(erfcf(0x1.c00002p+1f), 0x1.8ef27c5661bc5p-21, 1) && within_ulps(erfcf(-0x1.f33332p+1f), 0x1.ffffff6a91796p+0, 1));
	QUIET(within_ulps(erfcf(-0x1.f33336p+1f), 0x1.ffffff6a919efp+0, 1));
	/*
	 * regression: erfcf correctly rounded (every build type's is) where the float path's first builds were not: 1.06 ulp
	 * from rounding c1 t with the rest, 1.76 from the product's missing e_lo r_lo, and the pair's worst points before c4
	 * became a pair (P11.04d2)
	 */
	QUIET(erfcf(0x1.0d79e6p-1f) == 0x1.d3a322p-2f && erfcf(0x1.ff13b2p-2f) == 0x1.ebd1d6p-2f && erfcf(0x1.042442p-9f) == 0x1.feda76p-1f && erfcf(0x1.89181p-14f) == 0x1.fff224p-1f);
	QUIET(erfcf(0x1.015546p+3f) == 0x1.d06436p-98f && erfcf(-0x1.ff4468p-2f) == 0x1.85164p+0f && erfcf(0x1.98902ep-4f) == 0x1.c69054p-1f);
	/* The subnormal results near 9.2 and 10 (underflow is raised, so accuracy only) */
	CHECK(within_ulps(erfcf(0x1.266664p+3f), 0x1.cee509d8309dp-127, 1) && within_ulps(erfcf(0x1.266668p+3f), 0x1.cee0dafd0fa85p-127, 1) && within_ulps(erfcf(0x1.3ffffep+3f), 0x1.7d8c5e7556c84p-149, 1) && within_ulps(erfcf(0x1.400002p+3f), 0x1.7d889fe214adbp-149, 1));
}

/* x is want, the correctly rounded double, or a neighbour of it: within 1 ulp of it (the double fast bound) */
static bool near_double(double x, double want)
{
	return x == want || x == nextafter(want, INFINITY) || x == nextafter(want, -INFINITY);
}

static void check_double_exponentials(void)
{
	/*
	 * P11.05b (the double-FPU cores' double-only exp family, D187; the strict files elsewhere): the special values, each side
	 * of the subnormal scaling from 2^-1014 and of the ends, exp10's exact powers, and the sweeps' regressions: the unscaled
	 * product went subnormal near 2^-1020 (0.76 ulp), exp10's end was log10 2^-1074 (1 ulp), and expm1's terms cancelled
	 * near ln 2/64 (2.1 ulp)
	 */
	QUIET(exp(0.0) == 1.0 && exp(-INFINITY) == 0.0 && exp(INFINITY) == INFINITY && isnan(exp(NAN)) && exp2(-1074.0) == 0x1p-1074 && exp2(1023.0) == 0x1p1023 && exp10(22.0) == 1e22);
	QUIET(exp2(-INFINITY) == 0.0 && exp10(-INFINITY) == 0.0 && expm1(-INFINITY) == -1.0 && is_negative_zero((float)expm1(-0.0)) && exp10(3.0) == 1000.0);
	CHECK(near_double(exp(-0x1.62552039fe9f2p+9), 0x0.c3b18d96dcfa3p-1022) && near_double(exp(-0x1.6232bdd7abcd2p+9), 0x1.000000000007cp-1022) && exp(-0x1.74910d52d3051p+9) <= 0x1p-1074);
	QUIET(near_double(exp(0x1.62e42fefa39efp+9), 0x1.fffffffffff2ap+1023) && near_double(exp2(-0x1.fe00000000001p+9), 0x1.ffffffffffd3ap-1021) && near_double(exp2(0x1.fffffffffffffp+9), 0x1.ffffffffffd3ap+1023));
	CHECK(near_double(exp2(-0x1.ff2593ac77001p+9), 0x0.d0dd95bcd370fp-1022) && exp10(-0x1.439b746e36b52p+8) == 0x1p-1074 && exp10(-0x1.434e642d0efc8p+8) == 0x1p-1074);
	QUIET(near_double(exp10(0x1.34413509f79fep+8), 0x1.ffffffffffba1p+1023) && near_double(expm1(0x1.62f0eeacb7p-7), 0x1.64ded6012030ap-7) && near_double(expm1(-0x1.64aaac4f09a7ep-7), -0x1.62bb8d795fbfap-7));
	QUIET(near_double(expm1(0x1.00ce25a05127p+9), 0x1.fa20715abe035p+740) && near_double(expm1(0x1.6232bdd7abcd2p-7), 0x1.641e9377fffb7p-7));
	RAISES(exp(710.0) == INFINITY && exp2(1024.0) == INFINITY && exp10(309.0) == INFINITY && expm1(710.0) == INFINITY, FE_OVERFLOW);
	/*
	 * P11.05c1 (the soft-double cores' fixed-point kernel, D188): the rounding's carry into the exponent just below 2, each
	 * side of the fraction's shifts at 2^-12 and of the common path's ends at 2^-54 and 512 (256 for exp10), and the
	 * sweeps' worst points
	 */
	QUIET(near_double(exp2(0x1.fffffffffffffp-1), 0x1.fffffffffffffp+0) && near_double(exp2(-0x1.fffffffffffffp-1), 0x1p-1) && exp2(0x1p-54) == 1.0);
	QUIET(near_double(exp2(0x1.0000000000001p-13), 0x1.00058ba01fba0p+0) && near_double(exp2(0x1.fffffffffffffp-13), 0x1.000b175effdc7p+0) && near_double(exp2(0x1.fffffffffffffp+8), 0x1.ffffffffffe9dp+511));
	QUIET(near_double(exp2(-0x1.fffffffffffffp+8), 0x1.00000000000b1p-512) && near_double(exp(0x1.fffffffffffffp+8), 0x1.9476504ba839ap+738) && near_double(exp(-0x1.fffffffffffffp+8), 0x1.44109edb20a75p-739));
	QUIET(exp(0x1p-54) == 1.0 && near_double(exp(-0x1.b7f4d3965907fp-49), 0x1.fffffffffffe5p-1) && near_double(exp(0x1.62e42fefa39efp-1), 2.0) && exp10(1.0) == 10.0);
	QUIET(near_double(exp10(0x1.fffffffffffffp+7), 0x1.54fdd7f73bdb3p+850) && near_double(exp10(-0x1.fffffffffffffp+7), 0x1.8062864ac70fep-851) && near_double(exp10(0x1.29b7640c6aae2p+8), 0x1.fd4d1534d970dp+988));
	/*
	 * P11.05c2 (the soft-double cores' fixed-point expm1, D189): each side of the series' end at 1/16, of 2^-54 and 512, of
	 * 64 ln 2 (where the 1 drops below a unit of the kernel's result) and of -40, and the sweeps' worst points
	 */
	QUIET(near_double(expm1(0x1.fffffffffffffp-5), 0x1.082b577d34ed7p-4) && near_double(expm1(0x1p-4), 0x1.082b577d34ed8p-4) && near_double(expm1(-0x1.fffffffffffffp-5), -0x1.f0540438fd5c2p-5));
	QUIET(near_double(expm1(-0x1p-4), -0x1.f0540438fd5c3p-5) && expm1(0x1p-54) == 0x1p-54 && expm1(-0x1p-54) == -0x1p-54 && near_double(expm1(1.0), 0x1.b7e151628aed3p+0) && near_double(expm1(-1.0), -0x1.43a54e4e98864p-1));
	QUIET(near_double(expm1(0x1.62e42fefa39efp+5), 0x1.ffffffffffff3p+63) && near_double(expm1(0x1.62e42fefa39eep+5), 0x1.fffffffffffb3p+63) && near_double(expm1(0x1.fffffffffffffp+8), 0x1.9476504ba839ap+738));
	CHECK(expm1(-0x1.3ffffffffffffp+5) == -1.0 && expm1(-40.0) == -1.0 && expm1(-0x1.fffffffffffffp+8) == -1.0 && expm1(-512.0) == -1.0);
	QUIET(near_double(expm1(-0x1.0a60a68030f0fp-4), -0x1.01e68a9e8cbep-4) && near_double(expm1(-0x1.045d580e1e606p-4), -0x1.f886318541a1fp-5) && near_double(expm1(0x1.36290a8f9f8f1p-4), 0x1.42349c9a9462dp-4));
	QUIET(near_double(expm1(-0x1.00000000cd18fp-4), -0x1.f054043a7eb3dp-5) && near_double(expm1(0x1.5bcfa7205d3d6p-5), 0x1.634ce30cbc108p-5) && near_double(expm1(-0x1.0c7f173a14fdap-5), -0x1.0824e299f4421p-5));
}

static void check_double_logarithms(void)
{
	/*
	 * P11.05d (the double-FPU cores' double-only logarithms, D190; the strict files elsewhere): the special values, +0 for 1
	 * in every direction, the exact powers of 2 and 10, each side of the table's ends (0x1.68aaap-1 and its double) and of
	 * log1p's series at 2^-7.5, the extremes, the sweeps' worst points, and the regression: 2^-1023, scaled from a
	 * subnormal, has the bit pattern 0, which a first build took for a special value (log2 returned the argument)
	 */
	QUIET(log(1.0) == 0.0 && !signbit(log(1.0)) && log2(1.0) == 0.0 && !signbit(log2(1.0)) && log10(1.0) == 0.0 && !signbit(log10(1.0)));
	QUIET(log(INFINITY) == INFINITY && isnan(log(NAN)) && log2(0x1p-1074) == -1074.0 && log2(0x1p-1023) == -1023.0 && log2(0x1p-1022) == -1022.0 && log2(0x1p1023) == 1023.0);
	QUIET(log10(1000.0) == 3.0 && log10(1e22) == 22.0 && log10(10.0) == 1.0 && log1p(0.0) == 0.0 && is_negative_zero((float)log1p(-0.0)) && log1p(INFINITY) == INFINITY);
	RAISES(log(0.0) == -INFINITY && log(-0.0) == -INFINITY && log2(0.0) == -INFINITY && log10(-0.0) == -INFINITY && log1p(-1.0) == -INFINITY, FE_DIVBYZERO);
	RAISES(isnan(log(-1.0)) && isnan(log2(-INFINITY)) && isnan(log10(-0x1p-1074)) && isnan(log1p(-2.0)) && isnan(log1p(-INFINITY)), FE_INVALID);
	QUIET(near_double(log(0x1.00b52855823a7p+0), 0x1.69d0b54f8dee1p-9) && near_double(log2(0x1.00ade07cc039ep+0), 0x1.f509cf6cd471fp-9) && near_double(log10(0x1.00ba421879db7p+0), 0x1.431acc7ff8b0fp-10));
	QUIET(near_double(log(0x1.68aaap-1), -0x1.66c7a173b5128p-2) && near_double(log(0x1.68a9fffffffffp-1), -0x1.66c967b93d82dp-2) && near_double(log(0x1.68aaap+0), 0x1.5f00be6b922b7p-2) && near_double(log(0x1.68a9fffffffffp+0), 0x1.5efef82609bb2p-2));
	QUIET(near_double(log(0x1.fffffffffffffp+1023), 0x1.62e42fefa39efp+9) && near_double(log(0x1p-1074), -0x1.74385446d71c3p+9) && near_double(log(0x1.fffffffffffffp-1), -0x1p-53) && near_double(log(0x1.0000000000001p+0), 0x1.fffffffffffffp-53));
	QUIET(near_double(log2(0x1.fffffffffffffp-1), -0x1.71547652b82fep-53) && near_double(log2(0x1.0000000000001p+0), 0x1.71547652b82fdp-52) && near_double(log10(0x1.0000000000001p+0), 0x1.bcb7b1526e50dp-54) && near_double(log10(0x1p-1074), -0x1.434e6420f4374p+8));
	QUIET(near_double(log1p(0x1.6a09e667f3bcdp-8), 0x1.690ad6c502fa9p-8) && near_double(log1p(0x1.6a09e667f3bcep-8), 0x1.690ad6c502faap-8) && near_double(log1p(-0x1.6a09e667f3bcdp-8), -0x1.6b0ad8c505a54p-8) && near_double(log1p(-0x1.6a09e667f3bcep-8), -0x1.6b0ad8c505a55p-8));
	QUIET(near_double(log1p(-0x1.ac44dfb0777p-9), -0x1.acf86168cd623p-9) && near_double(log1p(-0x1.fffffffffffffp-1), -0x1.25e4f7b2737fap+5) && near_double(log1p(0x1p53), 0x1.25e4f7b2737fap+5));
	/* Not QUIET: the strict file raises a spurious underflow there (its correction 1/x is subnormal) */
	CHECK(near_double(log1p(0x1.fffffffffffffp+1023), 0x1.62e42fefa39efp+9) && near_double(log1p(0x1p60), 0x1.4cb5ecf0a9650p+5) && near_double(log1p(0x1.0000000000001p60), 0x1.4cb5ecf0a9650p+5));
	QUIET(near_double(log1p(1.0), 0x1.62e42fefa39efp-1) && near_double(log1p(-0x1.6a09e667b433dp-8), -0x1.6b0ad8c4c5c1fp-8) && log1p(0x1p-61) == 0x1p-61);
	CHECK(log1p(-0x1p-1074) == -0x1p-1074 && log1p(0x1p-1074) == 0x1p-1074);
	/*
	 * P11.05e (the soft-double cores' fixed-point logarithms, D192): each side of the series' band around 1 (open at
	 * 1 - 2^-5 and 1 + 2^-5) and of its table's ends (45/64 and 90/64), log1p each side of 2^-5 and of the carry into a bit
	 * 64 at 2^63, the sweeps' worst points, and the regression: 1 - 2^-5 itself went to the series with |s| = 2^-5, where
	 * the series' shift is negative
	 */
	QUIET(near_double(log(0x1.fp-1), -0x1.0415d89e74444p-5) && near_double(log(0x1.f000000000001p-1), -0x1.0415d89e74434p-5) && near_double(log(0x1.08p+0), 0x1.f829b0e7833p-6) && near_double(log(0x1.07fffffffffffp+0), 0x1.f829b0e7832c2p-6));
	QUIET(near_double(log2(0x1.fp-1), -0x1.77394c9d958d5p-5) && near_double(log2(0x1.08p+0), 0x1.6bad3758efd87p-5) && near_double(log10(0x1.fp-1), -0x1.c3d0837784c41p-7) && near_double(log10(0x1.08p+0), 0x1.b5e908eb1379p-7));
	QUIET(near_double(log(0x1.68p-1), -0x1.68ac83e9c6a14p-2) && near_double(log(0x1.67fffffffffffp-1), -0x1.68ac83e9c6a17p-2) && near_double(log(0x1.68p+0), 0x1.5d1bdbf5809cap-2) && near_double(log(0x1.67fffffffffffp+0), 0x1.5d1bdbf5809c7p-2));
	QUIET(near_double(log10(0x1.08000000e5ae8p+0), 0x1.b5e9091b707f7p-7) && near_double(log10(0x1.efffffffa1775p-1), -0x1.c3d083821d15p-7) && near_double(log2(0x1.efffffff77c6bp-1), -0x1.77394caa43758p-5));
	QUIET(near_double(log1p(0x1p-5), 0x1.f829b0e7833p-6) && near_double(log1p(0x1.fffffffffffffp-6), 0x1.f829b0e7832ffp-6) && near_double(log1p(-0x1p-5), -0x1.0415d89e74444p-5) && near_double(log1p(-0x1.fffffffffffffp-6), -0x1.0415d89e74444p-5));
	QUIET(near_double(log1p(0x1.fffffffffffffp+62), 0x1.5d589f2fe5107p+5) && near_double(log1p(0x1p63), 0x1.5d589f2fe5107p+5) && near_double(log1p(0x1p64), 0x1.62e42fefa39efp+5) && near_double(log1p(0x1.6ap-1), 0x1.11cb81787ccf8p-1));
}

static void check_double_powers(void)
{
	/*
	 * P11.05f (the double-FPU cores' double-only pow and cbrt, D193; the strict files elsewhere): the special values of
	 * F.10.4.5, exact powers, each side of the ends and of the scaled path, a subnormal x and result, |y| from 2^63 and
	 * below 2^-65, x near 1 with a large y, the sweeps' worst point, and the regressions: the log pair's low part (r^3 Q,
	 * up to 2^-16 of it) pushed exp's reduced argument out of its range (130 ulp), and y log|x| just above ln(DBL_MAX)
	 * wrapped the scale's exponent into the sign
	 */
	QUIET(pow(2.0, 10.0) == 1024.0 && pow(-2.0, 3.0) == -8.0 && pow(-2.0, 2.0) == 4.0 && pow(NAN, 0.0) == 1.0 && pow(1.0, NAN) == 1.0 && pow(-1.0, INFINITY) == 1.0);
	QUIET(pow(0.5, INFINITY) == 0.0 && pow(2.0, -INFINITY) == 0.0 && pow(0.5, -INFINITY) == INFINITY && pow(-INFINITY, 3.0) == -INFINITY && pow(-INFINITY, -3.0) == 0.0 && signbit(pow(-INFINITY, -3.0)));
	QUIET(pow(-0.0, 3.0) == 0.0 && signbit(pow(-0.0, 3.0)) && pow(-0.0, 2.0) == 0.0 && !signbit(pow(-0.0, 2.0)) && pow(-1.0, 0x1p63) == 1.0 && pow(2.0, 0x1p-70) == 1.0);
	RAISES(pow(0.0, -1.0) == INFINITY && pow(-0.0, -1.0) == -INFINITY && pow(-0.0, -2.0) == INFINITY, FE_DIVBYZERO);
	RAISES(isnan(pow(-8.0, 1.0 / 3.0)) && isnan(pow(-0x1p-1074, 0.5)), FE_INVALID);
	RAISES(pow(2.0, 1024.0) == INFINITY && pow(10.0, 309.0) == INFINITY && pow(1.0000001, 0x1p63) == INFINITY && pow(0x1.fffff8bef5d93p-1, -0x1.99c2ea2d1d8cp+31) == INFINITY, FE_OVERFLOW);
	RAISES(pow(2.0, -1076.0) == 0.0 && pow(0.9999999, 0x1p63) == 0.0 && pow(-0.9999999, 0x1p63) == 0.0, FE_UNDERFLOW);
	QUIET(near_double(pow(0x1p-1074, 0.5), 0x1p-537) && near_double(pow(1.0 + 0x1p-52, 0x1p52), 0x1.5bf0a8b145769p+1) && near_double(pow(1.0 - 0x1p-53, 0x1p53), 0x1.78b56362cef37p-2));
	QUIET(near_double(pow(10.0, 308.0), 0x1.1ccf385ebc8ap+1023) && near_double(pow(0x1.6c9f7067e16c1p+0, -0x1.4a45dde4e60ddp+10), 0x1.dd328471f8c79p-675) && near_double(pow(0x1.00b320f51fe56p+0, 0x1.d98646a264519p+17), 0x1.a7e4afc6ceeecp+954));
	QUIET(near_double(pow(2.0, 0.5), 0x1.6a09e667f3bcdp+0) && near_double(pow(1.5, 2.5), 0x1.60b9fd68a4554p+1) && near_double(pow(3.0, -600.0), 0x1.0406143142763p-951) && near_double(pow(0x1.fffffffffffffp-1, -0x1p62), 0x1.9476504ba85f9p+738));
	QUIET(near_double(pow(2.0, 1023.5), 0x1.6a09e667f3bcdp+1023) && near_double(pow(-3.0, -600.0), 0x1.0406143142763p-951) && near_double(pow(-3.0, -601.0), -0x1.5ab2c59703484p-953));
	CHECK(near_double(pow(0.5, 1073.5), 0x1p-1074) && pow(0.5, 1074.0) == 0x1p-1074);
	/*
	 * P11.05g1 (the soft-double cores' fixed-point pow, D194): each side of the band around 1 and of the table's ends,
	 * subnormal results down to 2^-1074, a negative x to a large odd power, the sweeps' worst points, and the
	 * regressions: k went into Q64's bit, not Q96's (2^10 came out as 1), and subnormal results rounded twice (0.75 ulp)
	 */
	QUIET(pow(2.0, 10.0) == 1024.0 && pow(0.5, -3.0) == 8.0 && near_double(pow(0x1.fp-1, 100.0), 0x1.566bffb174bf5p-5) && near_double(pow(0x1.f000000000001p-1, 100.0), 0x1.566bffb174c3ap-5));
	QUIET(near_double(pow(0x1.08p+0, -100.0), 0x1.79911928de471p-5) && near_double(pow(0x1.07fffffffffffp+0, -100.0), 0x1.79911928de5p-5) && near_double(pow(0x1.68p-1, 3.5), 0x1.2a7aa2aacb885p-2) && near_double(pow(0x1.68p+0, -7.25), 0x1.59dd6e0c7039p-4));
	QUIET(near_double(pow(0x1.0000000000012p+0, -0x1.e1e36a3a0fbap+5), 0x1.ffffffffff788p-1) && near_double(pow(-0x1.400c1d6b00184p+1, 34.0), 0x1.ef85f07191b97p+44) && near_double(pow(-3.0, 645.0), -0x1.3b59c25079105p+1022));
	CHECK(near_double(pow(0x1.20ddfd6041bcp-1, 0x1.3574545c1b567p+10), 0x0.ecc4ccd5bf687p-1022) && near_double(pow(2.0, -1024.5), 0x0.2d413cccfe77ap-1022) && pow(2.0, -1074.5) == 0x1p-1074);
	QUIET(cbrt(27.0) == 3.0 && cbrt(-8.0) == -2.0 && cbrt(1000.0) == 10.0 && cbrt(0.0) == 0.0 && signbit(cbrt(-0.0)) && cbrt(INFINITY) == INFINITY && cbrt(-INFINITY) == -INFINITY && isnan(cbrt(NAN)));
	QUIET(near_double(cbrt(2.0), 0x1.428a2f98d728bp+0) && near_double(cbrt(4.0), 0x1.965fea53d6e3dp+0) && near_double(cbrt(0x1.fffffffffffffp+2), 2.0) && near_double(cbrt(0x1.fffffffffffffp+1023), 0x1.428a2f98d728bp+341));
	QUIET(near_double(cbrt(0x1p-1074), 0x1p-358) && near_double(cbrt(-0x1.d341a3e2ecb4dp-1009), -0x1.f0a0ec2378bccp-337) && near_double(cbrt(0x1.0000000000001p+0), 1.0));
	/*
	 * P11.05g2 (the soft-double cores' fixed-point cbrt, D195): each side of 1, 2, 4 and 8, where j and the clamp of T below
	 * 2 change, the normal and subnormal ends, exact cubes (normal and subnormal), and the sweeps' worst points before
	 * the second residual (0.73 ulp)
	 */
	QUIET(cbrt(343.0) == 7.0 && cbrt(0x1.bp-1067) == 0x1.8p-356 && cbrt(-0x1.bp-1067) == -0x1.8p-356 && cbrt(-27.0 / 64.0) == -0.75);
	QUIET(near_double(cbrt(0x1.ffffffffffffdp+2), 0x1.fffffffffffffp+0) && near_double(cbrt(0x1.0000000000001p+3), 0x1p+1) && near_double(cbrt(0x1.fffffffffffffp-1), 0x1p+0) && near_double(cbrt(0x1.fffffffffffffp+0), 0x1.428a2f98d728bp+0));
	QUIET(near_double(cbrt(0x1.fffffffffffffp+1), 0x1.965fea53d6e3cp+0) && near_double(cbrt(0x1p-1022), 0x1.428a2f98d728bp-341) && near_double(cbrt(0x0.fffffffffffffp-1022), 0x1.428a2f98d728ap-341) && near_double(cbrt(0x1p-1073), 0x1.428a2f98d728bp-358));
	QUIET(near_double(cbrt(-0x1.fa7265fd9d029p+416), -0x1.fe2468c2b1ec5p+138) && near_double(cbrt(-0x1.c54bcf4dbdb0fp-1), -0x1.eba1effdff152p-1));
}

static void check_double_trigonometric(void)
{
	/*
	 * P11.05h (the double-FPU cores' sin, cos, sincos and tan, D196; the strict files elsewhere): the special values, each
	 * side of the tiny threshold 2^-27, of pi/64 (the first breakpoint) and of 2^20 (the slow path), the doubles next to
	 * pi/2, pi and 4 pi (next to a zero, the slow path), a negative x with the half turn set, a large argument, the sweeps'
	 * worst points, and the regression: the reduced argument's low part reached 2^-40 unnormalized (257 ulp near 2^20)
	 */
	double s;
	double c;

	QUIET(sin(0.0) == 0.0 && !signbit(sin(0.0)) && signbit(sin(-0.0)) && cos(0.0) == 1.0 && cos(-0.0) == 1.0 && signbit(tan(-0.0)) && tan(0.0) == 0.0);
	QUIET(isnan(sin(NAN)) && isnan(cos(NAN)) && isnan(tan(NAN)));
	RAISES(isnan(sin(INFINITY)) && isnan(cos(-INFINITY)) && isnan(tan(INFINITY)), FE_INVALID);
	QUIET(sin(0x1.fffffffffffffp-28) == 0x1.fffffffffffffp-28 && near_double(sin(0x1p-27), 0x1p-27) && near_double(cos(0x1p-27), 1.0) && near_double(tan(0x1p-27), 0x1p-27));
	QUIET(near_double(sin(0x1.921fb54442d18p-6), 0x1.92155f7a3667ep-6) && near_double(sin(0x1.921fb54442d19p-6), 0x1.92155f7a3667fp-6) && near_double(cos(0x1.921fb54442d19p-6), 0x1.ffd886084cd0dp-1) && near_double(tan(0x1.921fb54442d19p-6), 0x1.92346247a91f1p-6));
	QUIET(near_double(sin(0x1.921fb54442d18p+1), 0x1.1a62633145c07p-53) && near_double(cos(0x1.921fb54442d18p+0), 0x1.1a62633145c07p-54) && near_double(tan(0x1.921fb54442d18p+0), 0x1.d02967c31cdb5p+53) && near_double(tan(0x1.921fb54442d18p+1), -0x1.1a62633145c07p-53));
	QUIET(near_double(sin(0x1.921fb54442d18p+3), -0x1.1a62633145c07p-51) && near_double(cos(0x1.921fb54442d18p+3), 1.0) && near_double(tan(0x1.921fb54442d18p+3), -0x1.1a62633145c07p-51));
	QUIET(near_double(sin(0x1.fffffffffffffp+19), 0x1.526ccb2de52a8p-2) && near_double(cos(0x1.fffffffffffffp+19), 0x1.e33ada9352c61p-1) && near_double(tan(0x1.fffffffffffffp+19), 0x1.6692e575533f1p-2));
	QUIET(near_double(sin(0x1p20), 0x1.526ccb2fc8656p-2) && near_double(cos(0x1p20), 0x1.e33ada92fe2aep-1) && near_double(tan(0x1p20), 0x1.6692e5779206fp-2));
	QUIET(near_double(sin(-4.0), 0x1.837b9dddc1eaep-1) && near_double(cos(-4.0), -0x1.4eaa606db24c1p-1) && near_double(tan(-4.0), -0x1.2866f9be4de13p+0));
	QUIET(near_double(sin(1e300), -0x1.a2c16b010e385p-1) && near_double(cos(1e300), -0x1.2699022adc4c1p-1) && near_double(tan(1e300), 0x1.6be411f37ac77p+0) && near_double(sin(0x1.bd0a14269acdfp+707), 0x1.6e1cbd6c025f7p-1));
	QUIET(near_double(sin(0x1.a94666d39717ep+5), 0x1.f61dda514f931p-3) && near_double(cos(-0x1.dcb93aa159b5ap+10), -0x1.ff4b26db560c5p-1) && near_double(tan(0x1.8d7989a8f2812p+15), -0x1.fc69756b245cap+1));
	CHECK(near_double(sin(-0x1.fb530144d26dap+19), 0x1.922f9b04d95f5p-5) && near_double(cos(-0x1.fb530144d26dap+19), 0x1.ff61f13782899p-1) && near_double(tan(-0x1.fb530144d26dap+19), 0x1.92abe9b2589ffp-5));
	/* sincos equals sin and cos, bit for bit */
	sincos(-4.0, &s, &c);
	CHECK(s == sin(-4.0) && c == cos(-4.0));
	sincos(0x1.921fb54442d18p+1, &s, &c);
	CHECK(s == sin(0x1.921fb54442d18p+1) && c == cos(0x1.921fb54442d18p+1));
	sincos(1e300, &s, &c);
	CHECK(s == sin(1e300) && c == cos(1e300));
}

static void check_double_inverse_trigonometric(void)
{
	/*
	 * P11.05j (the double-FPU cores' asin, acos, atan and atan2, D198; the strict files elsewhere): the special values and
	 * the domain errors, each side of 1/2 (the half-angle forms), of 2^-27, of 1 and 1/32 (atan's inversion and first
	 * breakpoint), of 2^60, the doubles next to +-1, atan2's quadrants and each side of its fast path's edges (exponents 60
	 * apart, 2^-960 and 2^959), a subnormal quotient, and the sweeps' worst points
	 */
	QUIET(asin(1.0) == 0x1.921fb54442d18p+0 && asin(-1.0) == -0x1.921fb54442d18p+0 && acos(1.0) == 0.0 && !signbit(acos(1.0)) && acos(-1.0) == 0x1.921fb54442d18p+1);
	QUIET(signbit(asin(-0.0)) && asin(0.0) == 0.0 && signbit(atan(-0.0)) && atan(INFINITY) == 0x1.921fb54442d18p+0 && atan(-INFINITY) == -0x1.921fb54442d18p+0);
	QUIET(isnan(asin(NAN)) && isnan(acos(NAN)) && isnan(atan(NAN)) && isnan(atan2(NAN, 1.0)) && isnan(atan2(1.0, NAN)));
	RAISES(isnan(asin(0x1.0000000000001p+0)) && isnan(acos(-0x1.0000000000001p+0)) && isnan(asin(INFINITY)), FE_INVALID);
	QUIET(atan2(0.0, -1.0) == 0x1.921fb54442d18p+1 && atan2(-0.0, 1.0) == 0.0 && signbit(atan2(-0.0, 1.0)) && atan2(1.0, 0.0) == 0x1.921fb54442d18p+0 && atan2(-INFINITY, INFINITY) == -0x1.921fb54442d18p-1);
	QUIET(near_double(asin(0.5), 0x1.0c152382d7366p-1) && near_double(asin(0x1.0000000000001p-1), 0x1.0c152382d7367p-1) && near_double(asin(0x1.fffffffffffffp-2), 0x1.0c152382d7365p-1));
	QUIET(near_double(acos(0.5), 0x1.0c152382d7366p+0) && near_double(acos(0x1.0000000000001p-1), 0x1.0c152382d7365p+0) && near_double(acos(0x1.fffffffffffffp-2), 0x1.0c152382d7366p+0));
	QUIET(near_double(asin(0x1.fffffffffffffp-1), 0x1.921fb50442d18p+0) && near_double(acos(0x1.fffffffffffffp-1), 0x1p-26) && near_double(acos(-0x1.fffffffffffffp-1), 0x1.921fb52442d18p+1));
	QUIET(asin(0x1.fffffffffffffp-28) == 0x1.fffffffffffffp-28 && near_double(asin(0x1p-27), 0x1p-27) && near_double(acos(0x1p-27), 0x1.921fb52442d18p+0) && near_double(atan(0x1p-27), 0x1p-27));
	QUIET(near_double(atan(1.0), 0x1.921fb54442d18p-1) && near_double(atan(0x1.fffffffffffffp-1), 0x1.921fb54442d18p-1) && near_double(atan(0x1.0000000000001p+0), 0x1.921fb54442d19p-1));
	QUIET(near_double(atan(0x1p-5), 0x1.ffd55bba97625p-6) && near_double(atan(0x1.0000000000001p-5), 0x1.ffd55bba97627p-6) && near_double(atan(0x1.fffffffffffffp-6), 0x1.ffd55bba97624p-6) && near_double(atan(0x1.8p-4), 0x1.7ee182602f10fp-4));
	QUIET(near_double(atan(0x1p60), 0x1.921fb54442d18p+0) && near_double(atan(0x1.fffffffffffffp59), 0x1.921fb54442d18p+0) && near_double(atan(-1e10), -0x1.921fb543d4dep+0));
	QUIET(near_double(atan2(1.0, 1.0), 0x1.921fb54442d18p-1) && near_double(atan2(1.0, -1.0), 0x1.2d97c7f3321d2p+1) && near_double(atan2(-1.0, -1.0), -0x1.2d97c7f3321d2p+1) && near_double(atan2(3.0, -2.0), 0x1.145385fa3af71p+1));
	QUIET(near_double(atan2(0x1p60, 1.0), 0x1.921fb54442d18p+0) && near_double(atan2(0x1p61, 1.0), 0x1.921fb54442d18p+0) && near_double(atan2(1.0, 0x1p60), 0x1p-60) && near_double(atan2(1.0, 0x1p61), 0x1p-61) && near_double(atan2(1.0, -0x1p60), 0x1.921fb54442d18p+1));
	QUIET(near_double(atan2(0x1p-960, 0x1p-960), 0x1.921fb54442d18p-1) && near_double(atan2(0x1p959, 0x1p959), 0x1.921fb54442d18p-1) && near_double(atan2(0x1p-1000, 3.0), 0x1.5555555555555p-1002));
	QUIET(near_double(asin(0x1.101dc84c39c9ep-1), 0x1.1ee558c804933p-1) && near_double(asin(-0x1.122a045a5a2fp-1), -0x1.215125851920fp-1) && near_double(acos(0x1.1da5cc25957a2p-1), 0x1.f537266fed469p-1) && near_double(acos(-0x1.122a045a5a2fp-1), 0x1.1164240367b1p+1));
	QUIET(near_double(atan2(-0x1.1f9a4767316fbp+42, 0x1.48836ba1bb9d2p+47), -0x1.c0209cc7ff694p-6) && near_double(atan2(0x1.1c54ae6882b6p-2, 0x1.39bbd506ebeeap+3), 0x1.cfe4447f19c0fp-6));
	/*
	 * P11.05k1 (the soft-double cores' fixed-point atan and atan2, D199): each side of the breakpoint estimate's 1/32 (where
	 * j rounds to 0 or 1, the relative and the absolute paths), of an exponent gap of 6 (the aligned path's limit), y = x
	 * next to 1 (j = 16), the second quadrant, and the sweeps' worst points
	 */
	QUIET(near_double(atan(0x1.0005964d2f5c4p-5), 0x1.ffe0858a728c2p-6) && near_double(atan(0x1.73dd0ff7123bp-5), 0x1.739bc1a10005dp-5) && near_double(atan(0x1.fffffffffffffp+59), 0x1.921fb54442d18p+0));
	QUIET(near_double(atan2(0x1.8fe9ebad92749p-5, 0x1.8fda6e94d040bp+0), 0x1.ffe92bf6150f3p-6) && near_double(atan2(0x1p-6, 1.0), 0x1.fff555bbb729bp-7) && near_double(atan2(0x1.fffffffffffffp-7, 1.0), 0x1.fff555bbb729ap-7));
	QUIET(near_double(atan2(0x1p-7, 0x1.fffffffffffffp-1), 0x1.fffd555bbba98p-8) && near_double(atan2(-0x1.d5d1778f60ba2p-39, 0x1.0241fedebaed6p-6), -0x1.d1b5fcb9031b6p-33));
	QUIET(near_double(atan2(0x1.fp-1, -1.0), 0x1.2f9fdd46e309ap+1) && near_double(atan2(1.0, 0x1.fffffffffffffp-1), 0x1.921fb54442d19p-1));
	/*
	 * P11.05k2 (the soft-double cores' fixed-point asin and acos, D200): each side of 2^-60 (acos's pi/2), the doubles two
	 * ulps below 1 (the square root's smallest arguments), the half-angle form's middle, and the sweeps' worst points
	 */
	QUIET(near_double(acos(0x1p-60), 0x1.921fb54442d18p+0) && near_double(acos(0x1.fffffffffffffp-61), 0x1.921fb54442d18p+0) && near_double(asin(0x1p-60), 0x1p-60));
	QUIET(near_double(asin(0x1.ffffffffffffep-1), 0x1.921fb4e9c057fp+0) && near_double(acos(0x1.ffffffffffffep-1), 0x1.6a09e667f3bcdp-26) && near_double(acos(-0x1.ffffffffffffep-1), 0x1.921fb5170194bp+1));
	QUIET(near_double(asin(0.75), 0x1.b235315c680dcp-1) && near_double(acos(0.75), 0x1.720a392c1d955p-1) && near_double(acos(-0.75), 0x1.359d26f93b6c3p+1));
	QUIET(near_double(asin(0x1.99b41ee9b3ee8p-1), 0x1.daf2a6673616dp-1) && near_double(acos(0x1.a14710bf73068p-1), 0x1.3c745b0f762f5p-1) && near_double(acos(0x1.fffffbd411a02p-1), 0x1.056f08e97b2cp-11));
}

static void check_double_hyperbolic(void)
{
	/*
	 * P11.05l (the double-FPU cores' sinh, cosh, tanh, asinh, acosh and atanh, D201; the strict files elsewhere): the
	 * special values, the poles, overflow and the domain errors, each side of 2^-27, of the series' ends (1/2 for sinh, 1/4
	 * for tanh), of 22 (the kernel's end) and of 2^28 (asinh's and acosh's log 2|x|), the largest finite sinh and cosh, the
	 * doubles next to 1 for acosh and atanh, and the sweeps' worst points: sinh and tanh rounded the series' x^3 term (0.61
	 * and 0.56 ulp next to 1/2 and 1/4) and the kernel's (S1 + S2) u (0.54 and 0.556 just above them), and acosh compared
	 * a quiet NaN signalling (invalid)
	 */
	QUIET(sinh(0.0) == 0.0 && signbit(sinh(-0.0)) && sinh(-INFINITY) == -INFINITY && cosh(0.0) == 1.0 && cosh(-INFINITY) == INFINITY && tanh(INFINITY) == 1.0 && tanh(-INFINITY) == -1.0 && signbit(tanh(-0.0)));
	QUIET(signbit(asinh(-0.0)) && asinh(-INFINITY) == -INFINITY && acosh(1.0) == 0.0 && !signbit(acosh(1.0)) && acosh(INFINITY) == INFINITY && signbit(atanh(-0.0)));
	QUIET(isnan(sinh(NAN)) && isnan(cosh(NAN)) && isnan(tanh(NAN)) && isnan(asinh(NAN)) && isnan(acosh(NAN)) && isnan(atanh(NAN)));
	RAISES(sinh(0x1.633ce8fb9f87ep+9) == INFINITY && sinh(-711.0) == -INFINITY && cosh(-0x1.633ce8fb9f87ep+9) == INFINITY, FE_OVERFLOW);
	RAISES(isnan(acosh(0x1.fffffffffffffp-1)) && isnan(acosh(-INFINITY)) && isnan(atanh(0x1.0000000000001p+0)) && isnan(atanh(-INFINITY)), FE_INVALID);
	RAISES(atanh(1.0) == INFINITY && atanh(-1.0) == -INFINITY, FE_DIVBYZERO);
	RAISES(sinh(0x1p-1074) == 0x1p-1074 && tanh(-0x1p-1060) == -0x1p-1060 && asinh(0x1p-1050) == 0x1p-1050 && atanh(0x1p-1040) == 0x1p-1040, FE_UNDERFLOW);
	QUIET(near_double(sinh(0x1.0000000000000p-27), 0x1.0000000000000p-27) && near_double(sinh(0x1.0000000000001p-27), 0x1.0000000000001p-27) && near_double(sinh(0x1.0000000000000p-1), 0x1.0acd00fe63b97p-1) && near_double(sinh(0x1.fffffffffffffp-2), 0x1.0acd00fe63b96p-1));
	QUIET(near_double(sinh(-22.0), -0x1.ab5adb9c43600p+30) && near_double(sinh(0x1.6000000000001p+4), 0x1.ab5adb9c4361ap+30) && near_double(sinh(0x1.e566c45dc963cp-2), 0x1.f7c8f65b1b781p-2) && near_double(sinh(0x1.70bfdc8f25757p-1), 0x1.91772537e0223p-1));
	QUIET(near_double(sinh(0x1.daae675affd90p+8), 0x1.c40a38f952c41p+683) && near_double(sinh(-0x1.0070a9ed604c0p+9), -0x1.e7a6416fdb39dp+738) && near_double(sinh(-0x1.633ce8fb9f87dp+9), -0x1.ffffffffffd3bp+1023));
	QUIET(near_double(cosh(0x1.0000000000000p-27), 0x1.0000000000000p+0) && near_double(cosh(0x1.0000000000000p-1), 0x1.20ac1862ae8d0p+0) && near_double(cosh(0x1.fffffffffffffp-2), 0x1.20ac1862ae8d0p+0) && near_double(cosh(22.0), 0x1.ab5adb9c43600p+30));
	QUIET(near_double(cosh(0x1.6000000000001p+4), 0x1.ab5adb9c4361ap+30) && near_double(cosh(-0x1.412fa76365ae1p+4), 0x1.f24b4f98433adp+27) && near_double(cosh(-0x1.70c1114ec3a75p-1), 0x1.455105b77ebcbp+0) && near_double(cosh(0x1.633ce8fb9f87dp+9), 0x1.ffffffffffd3bp+1023));
	QUIET(near_double(cosh(0x1.0000000000000p-7), 0x1.00020000aaaacp+0) && near_double(cosh(0x1.62e42fefa39efp-8), 0x1.0000f5fe17617p+0));
	QUIET(near_double(tanh(0x1.0000000000000p-27), 0x1.0000000000000p-27) && near_double(tanh(0x1.0000000000000p-2), 0x1.f597ea69a1c86p-3) && near_double(tanh(0x1.fffffffffffffp-3), 0x1.f597ea69a1c85p-3) && near_double(tanh(-0x1.0507d2c659529p-2), -0x1.ff0a21fb59783p-3));
	QUIET(near_double(tanh(0x1.fc1fe8f68df40p-3), 0x1.f1f2e7b24701fp-3) && near_double(tanh(-0x1.4746787048c76p-2), -0x1.3c91afc43db57p-2) && near_double(tanh(0x1.5ffffffffffffp+4), 0x1.0000000000000p+0) && near_double(tanh(22.0), 0x1.0000000000000p+0));
	QUIET(near_double(tanh(0x1.8000000000000p+0), 0x1.cf6f9786df577p-1));
	QUIET(near_double(asinh(0x1.0000000000000p-27), 0x1.0000000000000p-27) && near_double(asinh(0x1.0000000000001p-27), 0x1.0000000000001p-27) && near_double(asinh(0x1.fffffffffffffp+27), 0x1.419ecb712c481p+4) && near_double(asinh(268435456.0), 0x1.419ecb712c481p+4));
	QUIET(near_double(asinh(-0x1.fffffffffffffp+1023), -0x1.633ce8fb9f87ep+9) && near_double(asinh(-0x1.89d82e37a1140p-9), -0x1.89d807608ad4bp-9) && near_double(asinh(0x1.6a9cec9847f96p-9), 0x1.6a9cce480077bp-9) && near_double(asinh(0x1.8000000000000p+0), 0x1.31dc0090b63d8p+0));
	QUIET(near_double(acosh(0x1.0000000000001p+0), 0x1.6a09e667f3bccp-26) && near_double(acosh(0x1.0000000000002p+0), 0x1.0000000000000p-25) && near_double(acosh(2.0), 0x1.5124271980435p+0) && near_double(acosh(0x1.fffffffffffffp+27), 0x1.419ecb712c481p+4));
	QUIET(near_double(acosh(268435456.0), 0x1.419ecb712c481p+4) && near_double(acosh(0x1.fffffffffffffp+1023), 0x1.633ce8fb9f87ep+9) && near_double(acosh(0x1.00003ecfe279fp+0), 0x1.66a9ac9f57e97p-9) && near_double(acosh(0x1.00003f79df1eep+0), 0x1.688db80250a35p-9));
	QUIET(near_double(atanh(0x1.0000000000000p-27), 0x1.0000000000000p-27) && near_double(atanh(0x1.0000000000001p-27), 0x1.0000000000001p-27) && near_double(atanh(0x1.0000000000000p-1), 0x1.193ea7aad030bp-1) && near_double(atanh(0x1.fffffffffffffp-1), 0x1.2b708872320e2p+4));
	QUIET(near_double(atanh(-0x1.fffffffffffffp-1), -0x1.2b708872320e2p+4) && near_double(atanh(-0x1.81addd31454f5p-10), -0x1.81adef6dfdbc5p-10) && near_double(atanh(0x1.6424848a9aee2p-10), 0x1.642492e6bd009p-10));
	/*
	 * P11.05m1 (the soft-double cores' fixed-point sinh, cosh and tanh, D202): each side of the series' ends (1/4 for sinh,
	 * 1/8 for tanh), the e^-|x| term's shift reaching 64, and the sweeps' worst points
	 */
	QUIET(near_double(sinh(0x1.0000000000000p-2), 0x1.02accd9d08102p-2) && near_double(sinh(0x1.fffffffffffffp-3), 0x1.02accd9d08101p-2) && near_double(sinh(-0x1.b05054d30c4bcp-2), -0x1.bd4568aed0954p-2) && near_double(sinh(-0x1.3683e36e6ec9ep-2), -0x1.3b4bc11f47872p-2));
	QUIET(near_double(cosh(-0x1.330cf48daaf9bp+2), 0x1.e4f101ed5a5f9p+5) && near_double(cosh(0x1.7343718b8e291p-1), 0x1.464ddfe8f0c2bp+0) && near_double(sinh(0x1.709d048b2a8c6p+8), 0x1.bcb061e40e697p+530) && near_double(cosh(0x1.709d048b2a8c6p+8), 0x1.bcb061e40e697p+530));
	QUIET(near_double(tanh(0x1.0000000000000p-3), 0x1.fd5992bc4b835p-4) && near_double(tanh(0x1.fffffffffffffp-4), 0x1.fd5992bc4b834p-4) && near_double(tanh(-0x1.789a1cb1d52dcp-3), -0x1.7469eb6a1fb28p-3) && near_double(tanh(0x1.367301cbbeb91p-3), 0x1.3417cff2bd6a4p-3));
	QUIET(near_double(tanh(0x1.5fffffffffff0p+4), 0x1.0000000000000p+0) && near_double(cosh(0x1.0000000000000p-26), 0x1.0000000000001p+0));
	/*
	 * P11.05m2 (the soft-double cores' fixed-point asinh, acosh and atanh, D203): each side of log1p's argument reaching
	 * 2^-5 (the relative series and the table path), of 1 (asinh) and 2 (acosh), where the arguments change form, the
	 * double above 1 (acosh's smallest argument), each side of 2^-12 (atanh's 1 - |x| exact), the doubles next to 1/2 and
	 * 1 (atanh), and the sweeps' worst points
	 */
	QUIET(near_double(asinh(0x1.f83e0f83e0f83p-6), 0x1.f829b0e7832ffp-6) && near_double(asinh(0x1.f83e0f83e0f84p-6), 0x1.f829b0e783300p-6) && near_double(asinh(0x1.f83e0f83e0f85p-6), 0x1.f829b0e783301p-6) && near_double(acosh(0x1.001f07c1f07c1p+0), 0x1.f829b0e782b5fp-6));
	QUIET(near_double(acosh(0x1.001f07c1f07c2p+0), 0x1.f829b0e78337ep-6) && near_double(acosh(0x1.001f07c1f07c3p+0), 0x1.f829b0e783b9ep-6) && near_double(atanh(0x1.f81f81f81f81fp-7), 0x1.f829b0e783300p-7) && near_double(atanh(0x1.f81f81f81f820p-7), 0x1.f829b0e783301p-7));
	QUIET(near_double(atanh(0x1.f81f81f81f821p-7), 0x1.f829b0e783302p-7) && near_double(asinh(0x1.0000000000000p+0), 0x1.c34366179d427p-1) && near_double(asinh(0x1.fffffffffffffp-1), 0x1.c34366179d426p-1) && near_double(acosh(0x1.0000000000000p+1), 0x1.5124271980435p+0));
	QUIET(near_double(acosh(0x1.fffffffffffffp+0), 0x1.5124271980434p+0) && near_double(acosh(0x1.0000000000001p+0), 0x1.6a09e667f3bccp-26) && near_double(atanh(0x1.0000000000000p-12), 0x1.0000005555559p-12) && near_double(atanh(0x1.fffffffffffffp-13), 0x1.0000005555558p-12));
	QUIET(near_double(atanh(0x1.0000000000000p-1), 0x1.193ea7aad030bp-1) && near_double(atanh(0x1.fffffffffffffp-1), 0x1.2b708872320e2p+4));
	QUIET(near_double(asinh(0x1.f90358c9aaf29p-6), 0x1.f8eee23cff5e9p-6) && near_double(acosh(0x1.001fe30a578e7p+0), 0x1.ff12d046a47f0p-6) && near_double(atanh(-0x1.f9cec4ddb450ep-7), -0x1.f9d90e06916abp-7) && near_double(asinh(-0x1.fe12135b34d32p-6), -0x1.fdfcfde287c82p-6));
}

static void check_double_error_function(void)
{
	/*
	 * P11.05n (the double-FPU cores' erf, D204; the strict file elsewhere): the special values, underflow for a subnormal x,
	 * each side of 2^-60 (the tiny path), 2^-28 (the strict file's), 1/2 (the series' end) and 6, the pieces' edges (m +-
	 * 1/8, a tie of the rounding to m), and the sweeps' worst points: x^3 p0 rounded with the rest of the series (0.76 ulp)
	 * and the tiny path from 2^-28 (0.53)
	 */
	QUIET(erf(0.0) == 0.0 && signbit(erf(-0.0)) && erf(INFINITY) == 1.0 && erf(-INFINITY) == -1.0 && isnan(erf(NAN)) && near_double(erf(6.0), 1.0) && near_double(erf(-1e300), -1.0));
	RAISES(erf(0x1p-1070) == 0x1.2p-1070 && erf(-0x1p-1074) == -0x1p-1074, FE_UNDERFLOW);
	QUIET(near_double(erf(0x1.0000000000000p-60), 0x1.20dd750429b6dp-60) && near_double(erf(0x1.fffffffffffffp-61), 0x1.20dd750429b6dp-60) && near_double(erf(0x1.0000000000000p-28), 0x1.20dd750429b6dp-28) && near_double(erf(0x1.fffffffffffffp-29), 0x1.20dd750429b6cp-28));
	QUIET(near_double(erf(0x1.0000000000000p-1), 0x1.0a7ef5c18edd2p-1) && near_double(erf(0x1.fffffffffffffp-2), 0x1.0a7ef5c18edd2p-1) && near_double(erf(0x1.4000000000000p-1), 0x1.3f196dcd0f135p-1) && near_double(erf(0x1.3ffffffffffffp-1), 0x1.3f196dcd0f134p-1));
	QUIET(near_double(erf(0x1.4000000000001p-1), 0x1.3f196dcd0f136p-1) && near_double(erf(0x1.c000000000000p-1), 0x1.91724951b8fc6p-1) && near_double(erf(0x1.2000000000000p+0), 0x1.c6dad2829ec62p-1) && near_double(erf(0x1.7000000000000p+1), 0x1.fff9ba420e834p-1));
	QUIET(near_double(erf(0x1.8000000000000p+1), 0x1.fffd1ac4135f9p-1) && near_double(erf(0x1.7800000000000p+2), 0x1.fffffffffffffp-1) && near_double(erf(0x1.7ffffffffffffp+2), 0x1.0000000000000p+0) && near_double(erf(-0x1.3333333333333p-2), -0x1.50838881dea0fp-2));
	QUIET(near_double(erf(0x1.1bb8874e36d08p+0), 0x1.c4144a3af4197p-1) && near_double(erf(0x1.1b66f7c6b691ap+0), 0x1.c3de52d6984dfp-1) && near_double(erf(-0x1.d59d70884fd86p-2), -0x1.eefc28d3f3ae9p-2) && near_double(erf(0x1.caa205731e295p-2), 0x1.e4e4f27a916c3p-2));
	/*
	 * P11.05o (the soft-double cores' fixed-point erf, D205): each side of the pieces' edges at 3/4, 1, 2 and 4 (the
	 * index from x's bits changes there), the last piece, and the sweeps' worst points
	 */
	QUIET(near_double(erf(0x1.8000000000000p-1), 0x1.6c1c9759d0e5fp-1) && near_double(erf(0x1.7ffffffffffffp-1), 0x1.6c1c9759d0e5fp-1) && near_double(erf(0x1.0000000000000p+0), 0x1.af767a741088bp-1) && near_double(erf(0x1.fffffffffffffp-1), 0x1.af767a741088ap-1));
	QUIET(near_double(erf(0x1.0000000000000p+1), 0x1.fd9ae142795e3p-1) && near_double(erf(0x1.fffffffffffffp+0), 0x1.fd9ae142795e3p-1) && near_double(erf(0x1.0000000000000p+2), 0x1.ffffff7b91176p-1) && near_double(erf(0x1.fffffffffffffp+1), 0x1.ffffff7b91176p-1));
	QUIET(near_double(erf(0x1.7000000000000p+2), 0x1.ffffffffffffcp-1) && near_double(erf(0x1.7ffffffffffffp+2), 0x1.0000000000000p+0) && near_double(erf(0x1.bf47914be88f1p+1), 0x1.ffffe606932f0p-1) && near_double(erf(-0x1.bf478084cca96p+1), -0x1.ffffe6067a7b7p-1));
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
	check_trigonometric();
	check_inverse_trigonometric();
	check_hyperbolic();
	check_error_functions();
	check_double_exponentials();
	check_double_logarithms();
	check_double_powers();
	check_double_trigonometric();
	check_double_inverse_trigonometric();
	check_double_hyperbolic();
	check_double_error_function();

	if (failures == 0)
		puts("pass: libm_thresholds");
	return failures == 0 ? 0 : 1;
}
