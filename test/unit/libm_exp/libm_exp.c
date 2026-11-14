/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_exp.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* The build type's expectations (strict/ or fast/, by include path: the module's makefile) */
#include "libm_expect.h"

/*
 * libm part 2 (P1.14, docs/research/libm-exp-log.md): exponentials,
 * logarithms, powers and roots. Reference vectors (exact values from
 * 200-digit decimal arithmetic) bound the error: at most 1 ulp for double
 * and correct rounding for float to nearest, 2 and 1 ulp in the directed
 * modes. Then the C23 special cases (Annex F.10.3, F.10.4), the C23
 * functions libc-test lacks, the exact cases (an inexact exception there
 * is allowed, Annex F.10), and the exceptions of
 * overflow, underflow and tiny arguments in every rounding direction the
 * core supports (FE_* values 0-3: nearest, upward, downward, toward zero).
 */

static int failures;

static void fail(int line, const char *what)
{
	printf("fail: line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

/* The exception bits in canonical FE_* values, whether or not this core defines the macros (D44) */
#define INVALID 0x01u
#define DIVBYZERO 0x02u
#define OVERFLOW 0x04u
#define UNDERFLOW 0x08u
#define INEXACT 0x10u

/* The condition holds and raised exactly these exceptions (those the core has) */
#define EXPECT(condition, flags)                                         \
	do {                                                                 \
		bool holds;                                                      \
		unsigned int raised;                                             \
                                                                         \
		feclearexcept(FE_ALL_EXCEPT);                                    \
		holds = (condition);                                             \
		raised = (unsigned int)fetestexcept(FE_ALL_EXCEPT);              \
		if (!holds || raised != ((flags) & (unsigned int)FE_ALL_EXCEPT)) \
			fail(__LINE__, #condition " raising " #flags);               \
	} while (false)

/* The condition holds in the rounding direction (skipped where the core has only nearest) */
#define IN_DIRECTION(direction, condition, flags) \
	do {                                          \
		if (fesetround(direction) == 0) {         \
			EXPECT(condition, flags);             \
			fesetround(0);                        \
		}                                         \
	} while (false)

/* EXPECT for a check of exact results through libm_exact: the fast contract allows inexact there (libm_expect.h) */
#define EXPECT_EXACT(condition, flags)                                                        \
	do {                                                                                      \
		bool holds;                                                                           \
		unsigned int raised;                                                                  \
                                                                                              \
		feclearexcept(FE_ALL_EXCEPT);                                                         \
		holds = (condition);                                                                  \
		raised = (unsigned int)fetestexcept(FE_ALL_EXCEPT) & libm_exact_flags();              \
		if (!holds || raised != ((flags) & (unsigned int)FE_ALL_EXCEPT & libm_exact_flags())) \
			fail(__LINE__, #condition " raising " #flags);                                    \
	} while (false)

enum function {
	EXP,
	EXP2,
	EXP10,
	EXPM1,
	EXP2M1,
	EXP10M1,
	LOG,
	LOG2,
	LOG10,
	LOG1P,
	LOG2P1,
	LOG10P1,
	CBRT,
	RSQRT,
	HYPOT,
	POW,
	POWR,
	POWN,
	ROOTN,
	COMPOUNDN,
	EXPF,
	EXP2F,
	EXP10F,
	EXPM1F,
	EXP2M1F,
	EXP10M1F,
	LOGF,
	LOG2F,
	LOG10F,
	LOG1PF,
	LOG2P1F,
	LOG10P1F,
	CBRTF,
	RSQRTF,
	HYPOTF,
	POWF,
	POWRF,
	POWNF,
	ROOTNF,
	COMPOUNDNF,
};

static const char *const names[] = { "exp", "exp2", "exp10", "expm1", "exp2m1", "exp10m1", "log", "log2", "log10", "log1p", "log2p1", "log10p1", "cbrt", "rsqrt", "hypot", "pow", "powr", "pown", "rootn", "compoundn" };

static const struct {
	enum function function;
	double x;
	double b;
	double hi;
	double lo;
} vectors[] = {
#include "vectors.inc"
};

static double call(enum function function, double x, double b)
{
	float xf = (float)x;
	float bf = (float)b;
	long long n = (long long)b;

	switch (function) {
		case EXP:
			return exp(x);
		case EXP2:
			return exp2(x);
		case EXP10:
			return exp10(x);
		case EXPM1:
			return expm1(x);
		case EXP2M1:
			return exp2m1(x);
		case EXP10M1:
			return exp10m1(x);
		case LOG:
			return log(x);
		case LOG2:
			return log2(x);
		case LOG10:
			return log10(x);
		case LOG1P:
			return log1p(x);
		case LOG2P1:
			return log2p1(x);
		case LOG10P1:
			return log10p1(x);
		case CBRT:
			return cbrt(x);
		case RSQRT:
			return rsqrt(x);
		case HYPOT:
			return hypot(x, b);
		case POW:
			return pow(x, b);
		case POWR:
			return powr(x, b);
		case POWN:
			return pown(x, n);
		case ROOTN:
			return rootn(x, n);
		case COMPOUNDN:
			return compoundn(x, n);
		case EXPF:
			return expf(xf);
		case EXP2F:
			return exp2f(xf);
		case EXP10F:
			return exp10f(xf);
		case EXPM1F:
			return expm1f(xf);
		case EXP2M1F:
			return exp2m1f(xf);
		case EXP10M1F:
			return exp10m1f(xf);
		case LOGF:
			return logf(xf);
		case LOG2F:
			return log2f(xf);
		case LOG10F:
			return log10f(xf);
		case LOG1PF:
			return log1pf(xf);
		case LOG2P1F:
			return log2p1f(xf);
		case LOG10P1F:
			return log10p1f(xf);
		case CBRTF:
			return cbrtf(xf);
		case RSQRTF:
			return rsqrtf(xf);
		case HYPOTF:
			return hypotf(xf, bf);
		case POWF:
			return powf(xf, bf);
		case POWRF:
			return powrf(xf, bf);
		case POWNF:
			return pownf(xf, n);
		case ROOTNF:
			return rootnf(xf, n);
		case COMPOUNDNF:
			return compoundnf(xf, n);
	}

	return NAN;
}

/* The error of got against the exact hi + lo, in units of the last place of the result format */
static double ulp_error(double got, double hi, double lo, bool single)
{
	int exponent = ilogb(hi);
	int minimum = single ? FLT_MIN_EXP - 1 : DBL_MIN_EXP - 1;
	int digits = single ? FLT_MANT_DIG : DBL_MANT_DIG;

	if (exponent < minimum)
		exponent = minimum;

	return fabs(((got - hi) - lo) / ldexp(1.0, exponent - digits + 1));
}

static void check_vectors(void)
{
	/* Nearest: 1 ulp for double, correct rounding for float; directed: 2 and 1 ulp (libm_expect.h) */
	double worst[40] = { 0 };

	for (int direction = 0; direction < 4; ++direction) {
		if (fesetround(direction) != 0)
			continue;
		for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
			bool single = vectors[i].function >= EXPF;
			double got = call(vectors[i].function, vectors[i].x, vectors[i].b);
			double error;

			fesetround(0);
			error = ulp_error(got, vectors[i].hi, vectors[i].lo, single);
			if (!libm_vector_within(names[vectors[i].function % 20], single, direction, error, got, vectors[i].hi)) {
				char message[200];

				snprintf(message, sizeof(message), "%s%s(%a, %a) direction %d = %a, want %a + %a: %g ulp", names[vectors[i].function % 20], single ? "f" : "", vectors[i].x, vectors[i].b, direction, got, vectors[i].hi, vectors[i].lo, error);
				fail(__LINE__, message);
			}
			if (direction == 0 && error > worst[vectors[i].function])
				worst[vectors[i].function] = error;
			fesetround(direction);
		}
	}
	fesetround(0);

	for (int i = 0; i < 40; ++i)
		printf("%s%s: %.3f ulp\n", names[i % 20], i >= 20 ? "f" : "", worst[i]);
}

static void check_exponentials(void)
{
	double tiny = 0x1p-1070;

	/* F.10.3.1: exp(+-0) is 1, exp(-inf) is +0, exp(+inf) is +inf, all exact */
	EXPECT(exp(0.0) == 1.0 && exp(-0.0) == 1.0, 0);
	EXPECT(exp(-INFINITY) == 0 && !signbit(exp(-INFINITY)) && exp(INFINITY) == INFINITY, 0);
	EXPECT(isnan(exp(NAN)), 0);
	EXPECT(exp(710.0) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(exp(-746.0) == 0, UNDERFLOW | INEXACT);
	EXPECT(libm_exact("exp", exp(-745.1), 0x1p-1074), UNDERFLOW | INEXACT);
	EXPECT(exp(-708.5) < DBL_MIN, UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("exp", exp(-1000.0), 0x1p-1074), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("exp", exp(-1000.0), 0.0) && !signbit(exp(-1000.0)), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("exp", exp(-744.8), 0.0) && !signbit(exp(-744.8)), UNDERFLOW | INEXACT);
	IN_DIRECTION(3, libm_directed("exp", exp(1000.0), DBL_MAX), OVERFLOW | INEXACT);
	IN_DIRECTION(1, exp(1000.0) == INFINITY, OVERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("exp", exp(0x1p-60), 1.0), INEXACT);
	IN_DIRECTION(1, libm_directed("exp", exp(0x1p-60), nextafter(1.0, 2.0)), INEXACT);
	CHECK(libm_exactf("exp", expf(-103.5f), 0x1p-149f) && expf(-104.0f) == 0 && expf(89.0f) == INFINITY);

	/* exp2 of an integer is exact; the halfway case below the subnormals rounds to even */
	for (int k = -1074; k <= 1023; k += 13)
		EXPECT_EXACT(libm_exact("exp2", exp2((double)k), ldexp(1.0, k)), 0);
	EXPECT(exp2(-1075.0) == 0, UNDERFLOW | INEXACT);
	EXPECT(exp2(1024.0) == INFINITY, OVERFLOW | INEXACT);
	EXPECT_EXACT(libm_exactf("exp2", exp2f(-149.0f), 0x1p-149f) && libm_exactf("exp2", exp2f(127.0f), 0x1p127f), 0);
	EXPECT(exp10(0.0) == 1.0 && exp10(-INFINITY) == 0, 0);
	for (int k = 1; k <= 22; ++k)
		CHECK(libm_exact("exp10", exp10((double)k), pown(10.0, k)));
	CHECK(libm_exact("exp10", exp10(-1.0), 0.1) && libm_exactf("exp10", exp10f(-1.0f), 0.1f) && libm_exact("exp10", exp10(308.0), 1e308));
	EXPECT(exp10(309.0) == INFINITY, OVERFLOW | INEXACT);

	/* F.10.3.4: expm1(+-0) is +-0, expm1(-inf) is -1; tiny arguments round in every direction */
	EXPECT(expm1(-0.0) == 0 && signbit(expm1(-0.0)), 0);
	EXPECT(expm1(-INFINITY) == -1.0 && expm1(INFINITY) == INFINITY, 0);
	EXPECT(libm_exact("expm1", expm1(-50.0), -1.0), INEXACT);
	IN_DIRECTION(1, libm_directed("expm1", expm1(-50.0), nextafter(-1.0, 0.0)), INEXACT);
	EXPECT(expm1(710.0) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(libm_exact("expm1", expm1(tiny), tiny), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("expm1", expm1(tiny), nextafter(tiny, 1.0)), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("expm1", expm1(-tiny), -tiny), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("expm1", expm1(0x1p-70), nextafter(0x1p-70, 1.0)), INEXACT);
	EXPECT(libm_exactf("expm1", expm1f(0x1p-140f), 0x1p-140f), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("expm1", expm1f(0x1p-140f), nextafterf(0x1p-140f, 1.0f)), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("expm1", expm1f(-0x1p-40f), -0x1p-40f), INEXACT);

	/* C23 7.12.6.5, 7.12.6.3: exp2m1 and exp10m1, exact at the integers */
	CHECK(libm_exact("exp2m1", exp2m1(1.0), 1.0) && libm_exact("exp2m1", exp2m1(-1.0), -0.5) && libm_exact("exp2m1", exp2m1(10.0), 1023.0));
	EXPECT(exp2m1(-0.0) == 0 && signbit(exp2m1(-0.0)) && exp2m1(-INFINITY) == -1.0, 0);
	EXPECT(exp2m1(1024.0) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(isnan(exp2m1(NAN)) && isnan(exp10m1(NAN)), 0);
	CHECK(libm_exact("exp10m1", exp10m1(1.0), 9.0) && libm_exact("exp10m1", exp10m1(2.0), 99.0) && exp10m1(-INFINITY) == -1.0);
	CHECK(libm_exact("exp10m1", exp10m1(-1.0), -0.9) && libm_exactf("exp2m1", exp2m1f(3.0f), 7.0f) && libm_exactf("exp10m1", exp10m1f(3.0f), 999.0f));
	CHECK(libm_exact("exp2m1", exp2m1(-0x1p-1060), -0x1p-1060 * 0x1.62e42fefa39efp-1) || exp2m1(-0x1p-1060) == nextafter(-0x1p-1060 * 0x1.62e42fefa39efp-1, 0));
	CHECK(libm_exact("exp2m1", (double)exp2m1l(4.0L), 15.0) && libm_exact("exp10m1", (double)exp10m1l(1.0L), 9.0) && expm1l(-INFINITY) == -1.0L);
}

static void check_logarithms(void)
{
	/* F.10.3.7: log(1) is +0 in every direction, log(+-0) is -inf with divide-by-zero */
	EXPECT(log(1.0) == 0 && !signbit(log(1.0)), 0);
	IN_DIRECTION(2, log(1.0) == 0 && !signbit(log(1.0)), 0);
	EXPECT(log(0.0) == -INFINITY, DIVBYZERO);
	EXPECT(log(-0.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(log(-1.0)), INVALID);
	EXPECT(isnan(log(-INFINITY)), INVALID);
	EXPECT(log(INFINITY) == INFINITY && isnan(log(NAN)), 0);
	CHECK(libm_exact("log", log(0x1.5bf0a8b145769p+1), 1.0) && libm_exact("log", log(0x1p-1074), -0x1.74385446d71c3p+9));

	/* log2 of a power of two is exact, as is log10 of a power of ten */
	for (int k = -1074; k <= 1023; ++k)
		EXPECT_EXACT(libm_exact("log2", log2(ldexp(1.0, k)), k), 0);
	for (int k = 0; k <= 22; ++k)
		CHECK(libm_exact("log10", log10(pown(10.0, k)), k));
	EXPECT(log2(0.0) == -INFINITY && log10(-0.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(log2(-0x1p-1074)) && isnan(log10f(-1.0f)), INVALID);
	EXPECT_EXACT(libm_exactf("log2", log2f(0x1p-149f), -149.0f) && libm_exact("log2", (double)log2l(0x1p1023L), 1023.0), 0);
	CHECK(libm_exactf("log10", log10f(1000.0f), 3.0f) && logf(1.0f) == 0);

	/* F.10.3.9: log1p(-1) is -inf with divide-by-zero, below -1 invalid; tiny arguments */
	EXPECT(log1p(-1.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(log1p(-2.0)) && isnan(log1p(-INFINITY)), INVALID);
	EXPECT(log1p(-0.0) == 0 && signbit(log1p(-0.0)) && log1p(INFINITY) == INFINITY, 0);
	EXPECT(libm_exact("log1p", log1p(0x1p-1070), 0x1p-1070), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("log1p", log1p(0x1p-1070), nextafter(0x1p-1070, 0.0)), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("log1p", log1p(-0x1p-1070), -0x1p-1070), UNDERFLOW | INEXACT);
	EXPECT(libm_exactf("log1p", log1pf(-0x1.fffffcp-127f), -0x1.fffffcp-127f), UNDERFLOW | INEXACT);
	IN_DIRECTION(3, libm_directed("log1p", log1pf(0x1p-140f), nextafterf(0x1p-140f, 0.0f)), UNDERFLOW | INEXACT);

	/* C23 7.12.6.15-17: logp1 is log1p; log2p1 and log10p1 exact where 1 + x is a power */
	for (double x = -0.75; x < 100; x += 0.37)
		CHECK(logp1(x) == log1p(x) && logp1f((float)x) == log1pf((float)x));
	CHECK(libm_exact("log2p1", log2p1(1.0), 1.0) && libm_exact("log2p1", log2p1(3.0), 2.0) && libm_exact("log2p1", log2p1(-0.5), -1.0) && libm_exact("log2p1", log2p1(0x1p60 - 1), 60.0));
	CHECK(libm_exact("log10p1", log10p1(9.0), 1.0) && libm_exact("log10p1", log10p1(99.0), 2.0) && libm_exact("log10p1", log10p1(999999.0), 6.0));
	EXPECT(log2p1(-1.0) == -INFINITY && log10p1(-1.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(log2p1(-1.5)) && isnan(log10p1f(-2.0f)), INVALID);
	EXPECT(log2p1(-0.0) == 0 && signbit(log2p1(-0.0)) && log10p1(INFINITY) == INFINITY, 0);
	CHECK(libm_exactf("log2p1", log2p1f(7.0f), 3.0f) && libm_exact("log10p1", (double)log10p1l(9.0L), 1.0) && logp1l(0.0L) == 0);
}

static void check_powers(void)
{
	/* F.10.4.5: pow's special cases */
	EXPECT(pow(-0.0, -3.0) == -INFINITY && pow(0.0, -3.0) == INFINITY, DIVBYZERO);
	EXPECT(pow(-0.0, -2.0) == INFINITY && pow(0.0, -0.5) == INFINITY, DIVBYZERO);
	EXPECT(pow(-0.0, 3.0) == 0 && signbit(pow(-0.0, 3.0)) && !signbit(pow(-0.0, 2.0)), 0);
	EXPECT(pow(-1.0, INFINITY) == 1.0 && pow(-1.0, -INFINITY) == 1.0, 0);
	EXPECT(pow(1.0, NAN) == 1.0 && pow(NAN, 0.0) == 1.0 && pow(NAN, -0.0) == 1.0, 0);
	EXPECT(isnan(pow(-2.0, 0.5)), INVALID);
	EXPECT(pow(0.5, -INFINITY) == INFINITY && pow(2.0, -INFINITY) == 0, 0);
	EXPECT(pow(0.5, INFINITY) == 0 && pow(2.0, INFINITY) == INFINITY, 0);
	EXPECT(pow(-INFINITY, -3.0) == 0 && signbit(pow(-INFINITY, -3.0)) && !signbit(pow(-INFINITY, -2.0)), 0);
	EXPECT(pow(-INFINITY, 3.0) == -INFINITY && pow(-INFINITY, 2.5) == INFINITY, 0);
	EXPECT(pow(INFINITY, -0.5) == 0 && pow(INFINITY, 0.5) == INFINITY, 0);

	/* Exact cases raise nothing; out of range raises overflow or underflow */
	EXPECT_EXACT(libm_exact("pow", pow(2.0, -1074.0), 0x1p-1074) && libm_exact("pow", pow(0x1p700, -1.5), 0x1p-1050) && libm_exact("pow", pow(4.0, 0.5), 2.0), 0);
	EXPECT_EXACT(libm_exact("pow", pow(0x1.8p-1070, 1.0), 0x1.8p-1070) && libm_exact("pow", pow(-0x1.4p-1072, 1.0), -0x1.4p-1072), 0);
	EXPECT_EXACT(libm_exact("pow", pow(0x1p-1042, 1.0), 0x1p-1042) && libm_exact("pow", pow(-2.0, 3.0), -8.0) && pow(0x1p-1074, -0.0) == 1.0, 0);
	EXPECT(pow(2.0, 1024.0) == INFINITY && pow(-2.0, 1025.0) == -INFINITY, OVERFLOW | INEXACT);
	EXPECT(pow(2.0, -1075.5) == 0 && pow(-2.0, -1077.0) == 0 && signbit(pow(-2.0, -1077.0)), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("pow", pow(-2.0, -1077.0), -0x1p-1074), UNDERFLOW | INEXACT);
	CHECK(libm_exact("pow", pow(9.0, 0.5), 3.0) && libm_exact("pow", pow(10.0, -1.0), 0.1) && libm_exact("pow", pow(1e10, 0.5), 1e5));
	CHECK(powf(-2.0f, -150.0f) == 0 && signbit(powf(-2.0f, -151.0f)) && libm_exact("pow", (double)powl(2.0L, 10.0L), 1024.0));

	/* C23 7.12.7.7, F.10.4.7: powr is exp(y log x) for x >= 0 */
	EXPECT(isnan(powr(-1.0, 2.0)) && isnan(powr(-0x1p-1074, 0.0)), INVALID);
	EXPECT(isnan(powr(0.0, 0.0)) && isnan(powr(INFINITY, -0.0)) && isnan(powr(1.0, INFINITY)), INVALID);
	EXPECT(isnan(powr(1.0, NAN)) && isnan(powr(NAN, 0.0)), 0);
	EXPECT(powr(1.0, 5.0) == 1.0 && powr(3.0, 0.0) == 1.0 && powr(0.0, 2.0) == 0, 0);
	EXPECT(powr(0.0, -1.0) == INFINITY && powr(-0.0, -3.0) == INFINITY, DIVBYZERO);
	EXPECT(powr(-0.0, -INFINITY) == INFINITY && powr(0.0, INFINITY) == 0, 0);
	EXPECT(powr(0.5, INFINITY) == 0 && powr(0.5, -INFINITY) == INFINITY && powr(INFINITY, -1.0) == 0, 0);
	CHECK(libm_exact("powr", powr(2.0, 0.5), 0x1.6a09e667f3bcdp+0) && libm_exactf("powr", powrf(8.0f, 1.0f / 3.0f), 2.0f) && libm_exact("powr", (double)powrl(4.0L, 1.5L), 8.0));

	/* C23 7.12.7.6: pown, exact for small results, pow's integer cases */
	CHECK(pown(NAN, 0) == 1.0 && libm_exact("pown", pown(2.0, 10), 1024.0) && libm_exact("pown", pown(-2.0, 3), -8.0));
	EXPECT(pown(-0.0, -3) == -INFINITY && pown(-0.0, -2) == INFINITY, DIVBYZERO);
	EXPECT(pown(-INFINITY, 3) == -INFINITY && pown(-INFINITY, -3) == 0 && signbit(pown(-INFINITY, -3)), 0);
	EXPECT(pown(-1.0, LLONG_MAX) == -1.0 && pown(-1.0, LLONG_MIN) == 1.0, 0);
	EXPECT(pown(2.0, -1075) == 0, UNDERFLOW | INEXACT);
	EXPECT(pown(2.0, LLONG_MAX) == INFINITY && pown(0.5, LLONG_MIN) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(pown(0.5, LLONG_MAX) == 0, UNDERFLOW | INEXACT);
	CHECK(libm_exact("pown", pown(10.0, -1), 0.1) && libm_exact("pown", pown(3.0, 20), 3486784401.0) && libm_exact("pown", pown(1.0000001, 1000000), pow(1.0000001, 1e6)));
	CHECK(libm_exactf("pown", pownf(-3.0f, 5), -243.0f) && libm_exact("pown", (double)pownl(0.5L, -3), 8.0));

	/* C23 7.12.7.8: rootn */
	EXPECT(isnan(rootn(2.0, 0)) && isnan(rootn(-4.0, 2)) && isnan(rootn(-INFINITY, -2)), INVALID);
	EXPECT(rootn(-0.0, 3) == 0 && signbit(rootn(-0.0, 3)) && !signbit(rootn(-0.0, 2)), 0);
	EXPECT(rootn(-0.0, -3) == -INFINITY && rootn(0.0, -2) == INFINITY && rootn(-0.0, -2) == INFINITY, DIVBYZERO);
	EXPECT(rootn(-INFINITY, 3) == -INFINITY && rootn(INFINITY, -2) == 0, 0);
	EXPECT_EXACT(libm_exact("rootn", rootn(5.5, 1), 5.5) && isnan(rootn(NAN, 3)), 0);
	CHECK(libm_exact("rootn", rootn(-8.0, 3), -2.0) && libm_exact("rootn", rootn(27.0, 3), 3.0) && libm_exact("rootn", rootn(1024.0, 10), 2.0) && libm_exact("rootn", rootn(0.25, -2), 2.0));
	CHECK(libm_exact("rootn", rootn(5.0, -1), 0.2) && libm_exact("rootn", rootn(0x1p-1074, 2), 0x1p-537) && libm_exactf("rootn", rootnf(-32.0f, 5), -2.0f) && libm_exact("rootn", (double)rootnl(81.0L, 4), 3.0));

	/* C23 7.12.7.9: rsqrt */
	CHECK(rsqrt(4.0) == 0.5 && rsqrt(0.25) == 2.0 && rsqrt(INFINITY) == 0);
	EXPECT(rsqrt(0.0) == INFINITY && rsqrt(-0.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(rsqrt(-1.0)) && isnan(rsqrtf(-INFINITY)), INVALID);
	CHECK(rsqrt(2.0) == 0x1.6a09e667f3bcdp-1 && rsqrtf(16.0f) == 0.25f && rsqrtl(0x1p-1074L) == 0x1p537L);

	/* C23 7.12.7.2: compoundn */
	EXPECT(compoundn(NAN, 0) == 1.0 && compoundn(-1.0, 0) == 1.0 && compoundn(INFINITY, 0) == 1.0, 0);
	EXPECT(isnan(compoundn(-2.0, 0)), INVALID);
	EXPECT(isnan(compoundn(-2.0, 1)) && isnan(compoundn(-INFINITY, 2)), INVALID);
	EXPECT(compoundn(-1.0, -1) == INFINITY, DIVBYZERO);
	EXPECT(compoundn(-1.0, 2) == 0 && compoundn(INFINITY, -1) == 0 && compoundn(INFINITY, 3) == INFINITY, 0);
	CHECK(libm_exact("compoundn", compoundn(1.0, 10), 1024.0) && libm_exact("compoundn", compoundn(0.5, 2), 2.25) && libm_exact("compoundn", compoundn(0.0, 1000), 1.0));
	EXPECT(compoundn(1.0, 1024) == INFINITY, OVERFLOW | INEXACT);
	CHECK(libm_exact("compoundn", compoundn(0.1, 1), 1.1) && libm_exact("compoundn", compoundn(-0.5, -3), 8.0) && libm_exactf("compoundn", compoundnf(1.0f, -2), 0.25f) && libm_exact("compoundn", (double)compoundnl(3.0L, 3), 64.0));
	/* A tiny x with a huge n: (1 + x)^n = e^(n x) is not 1 + n x (P2.05c3 regression) */
	CHECK(libm_near("compoundn", compoundn(0x1p-61, -(1LL << 61)), 0x1.78b56362cef38p-2, 0x1p-54) && libm_near("compoundn", compoundn(0x1p-70, 1LL << 62), 0x1.0100802ab5577p+0, 0x1p-52));
	CHECK(libm_near("compoundn", compoundn(0x1.8p-63, -0x7fffffffffffffffLL - 1), 0x1.c8f87724b5c1dp-3, 0x1p-55));
}

static void check_roots(void)
{
	/* F.10.4.1, F.10.4.3, F.10.4.10: cbrt, hypot and sqrt */
	CHECK(libm_exact("cbrt", cbrt(27.0), 3.0) && libm_exact("cbrt", cbrt(-8.0), -2.0) && libm_exact("cbrt", cbrt(0x1p-1074), 0x1p-358));
	EXPECT(cbrt(-0.0) == 0 && signbit(cbrt(-0.0)) && cbrt(-INFINITY) == -INFINITY && isnan(cbrt(NAN)), 0);
	CHECK(libm_exact("cbrt", cbrt(0x1p1023), 0x1p341) && libm_exact("cbrt", cbrt(2.0), 0x1.428a2f98d728bp+0) && libm_exactf("cbrt", cbrtf(-27.0f), -3.0f) && libm_exact("cbrt", (double)cbrtl(64.0L), 4.0));

	EXPECT(hypot(3.0, 4.0) == 5.0 && hypot(-5.0, 12.0) == 13.0 && hypot(-0.0, -0.0) == 0 && !signbit(hypot(-0.0, -0.0)), 0);
	EXPECT(hypot(INFINITY, NAN) == INFINITY && hypot(NAN, -INFINITY) == INFINITY && isnan(hypot(NAN, 1.0)), 0);
	EXPECT(hypot(3 * 0x1p-1074, 4 * 0x1p-1074) == 5 * 0x1p-1074 && hypot(-7.5, 0.0) == 7.5, 0);
	EXPECT(hypot(DBL_MAX, DBL_MAX) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(hypot(0x1.6666666666666p+1023, 0x1.3333333333333p+1023) == 0x1.d80a69c19e429p+1023, INEXACT);
	EXPECT(hypot(0x1p-1074, 0x1p-1074) == 0x1p-1074, UNDERFLOW | INEXACT);
	CHECK(hypot(1.0, 0x1p-30) == 1.0 && hypot(DBL_MAX, 1.0) == DBL_MAX && hypotf(3.0f, 4.0f) == 5.0f && hypotl(6.0L, 8.0L) == 10.0L);

	EXPECT(sqrt(-0.0) == 0 && signbit(sqrt(-0.0)) && sqrt(INFINITY) == INFINITY, 0);
	EXPECT(isnan(sqrt(-1.0)) && isnan(sqrtf(-0x1p-149f)) && isnan(sqrtl(-INFINITY)), INVALID);
	EXPECT(sqrt(0x1p-1074) == 0x1p-537 && sqrtf(0x1p-148f) == 0x1p-74f, 0);
	EXPECT(sqrt(DBL_MAX) == 0x1.fffffffffffffp+511, INEXACT);
	CHECK(sqrt(2.0) == 0x1.6a09e667f3bcdp+0 && sqrtf(2.0f) == 0x1.6a09e6p0f && sqrtl(9.0L) == 3.0L && dsqrtl(2.0L) == 0x1.6a09e667f3bcdp+0);
}

int main(void)
{
	check_vectors();
	check_exponentials();
	check_logarithms();
	check_powers();
	check_roots();

	if (failures == 0)
		printf("pass: libm_exp (%zu vectors)\n", sizeof(vectors) / sizeof(vectors[0]));

	return failures == 0 ? 0 : 1;
}
