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
	QUIET(within_ulps(erfcf(0x1.fffffep-27f), 0x1.ffffff6f91461p-1, 1) && within_ulps(erfcf(0x1.000002p-26f), 0x1.ffffff6f91446p-1, 1) && within_ulps(erfcf(0x1.fffffep-2f), 0x1.eb02163ed2107p-2, 1));
	QUIET(within_ulps(erfcf(0x1.000002p-1f), 0x1.eb0210f902b2fp-2, 1) && within_ulps(erfcf(-0x1.fffffep-2f), 0x1.853f7a704b7bep+0, 1) && within_ulps(erfcf(-0x1.000002p-1f), 0x1.853f7bc1bf534p+0, 1));
	QUIET(within_ulps(erfcf(0x1.fffffep-1f), 0x1.42261981e1c1dp-3, 1) && within_ulps(erfcf(0x1.000002p+0f), 0x1.42260f8b76284p-3, 1) && within_ulps(erfcf(0x1.fffffep+0f), 0x1.328f69582e052p-8, 1));
	QUIET(within_ulps(erfcf(0x1.000002p+1f), 0x1.328f499997a6ep-8, 1) && within_ulps(erfcf(0x1.7ffffep+1f), 0x1.729e1ad1530ffp-16, 1) && within_ulps(erfcf(0x1.800002p+1f), 0x1.729dd1cf18a16p-16, 1));
	QUIET(within_ulps(erfcf(0x1.bffffep+1f), 0x1.8ef2d6ecbe42cp-21, 1) && within_ulps(erfcf(0x1.c00002p+1f), 0x1.8ef27c5661bc5p-21, 1) && within_ulps(erfcf(-0x1.f33332p+1f), 0x1.ffffff6a91796p+0, 1));
	QUIET(within_ulps(erfcf(-0x1.f33336p+1f), 0x1.ffffff6a919efp+0, 1));
	/* The subnormal results near 9.2 and 10 (underflow is raised, so accuracy only) */
	CHECK(within_ulps(erfcf(0x1.266664p+3f), 0x1.cee509d8309dp-127, 1) && within_ulps(erfcf(0x1.266668p+3f), 0x1.cee0dafd0fa85p-127, 1) && within_ulps(erfcf(0x1.3ffffep+3f), 0x1.7d8c5e7556c84p-149, 1) && within_ulps(erfcf(0x1.400002p+3f), 0x1.7d889fe214adbp-149, 1));
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

	if (failures == 0)
		puts("pass: libm_thresholds");
	return failures == 0 ? 0 : 1;
}
