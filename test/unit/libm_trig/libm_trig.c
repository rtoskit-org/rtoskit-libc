/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_trig.c
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
 * libm part 3 (P1.15, docs/research/libm-trig.md): trigonometric,
 * pi-scaled and hyperbolic functions. Reference vectors (exact values from
 * 1400-bit mpmath, hard argument reductions included) bound the error: at most 1 ulp for double
 * and correct rounding for float to nearest, 2 and 1 ulp in the directed
 * modes. Then the C23 special cases (Annex F.10.1, F.10.2), the C23
 * pi functions libc-test lacks, the exact cases (an inexact exception there
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
	SIN,
	COS,
	TAN,
	ASIN,
	ACOS,
	ATAN,
	ATAN2,
	SINPI,
	COSPI,
	TANPI,
	ASINPI,
	ACOSPI,
	ATANPI,
	ATAN2PI,
	SINH,
	COSH,
	TANH,
	ASINH,
	ACOSH,
	ATANH,
	SINF,
	COSF,
	TANF,
	ASINF,
	ACOSF,
	ATANF,
	ATAN2F,
	SINPIF,
	COSPIF,
	TANPIF,
	ASINPIF,
	ACOSPIF,
	ATANPIF,
	ATAN2PIF,
	SINHF,
	COSHF,
	TANHF,
	ASINHF,
	ACOSHF,
	ATANHF,
};

static const char *const names[] = { "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinpi", "cospi", "tanpi", "asinpi", "acospi", "atanpi", "atan2pi", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh" };

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

	switch (function) {
		case SIN:
			return sin(x);
		case COS:
			return cos(x);
		case TAN:
			return tan(x);
		case ASIN:
			return asin(x);
		case ACOS:
			return acos(x);
		case ATAN:
			return atan(x);
		case ATAN2:
			return atan2(x, b);
		case SINPI:
			return sinpi(x);
		case COSPI:
			return cospi(x);
		case TANPI:
			return tanpi(x);
		case ASINPI:
			return asinpi(x);
		case ACOSPI:
			return acospi(x);
		case ATANPI:
			return atanpi(x);
		case ATAN2PI:
			return atan2pi(x, b);
		case SINH:
			return sinh(x);
		case COSH:
			return cosh(x);
		case TANH:
			return tanh(x);
		case ASINH:
			return asinh(x);
		case ACOSH:
			return acosh(x);
		case ATANH:
			return atanh(x);
		case SINF:
			return sinf(xf);
		case COSF:
			return cosf(xf);
		case TANF:
			return tanf(xf);
		case ASINF:
			return asinf(xf);
		case ACOSF:
			return acosf(xf);
		case ATANF:
			return atanf(xf);
		case ATAN2F:
			return atan2f(xf, bf);
		case SINPIF:
			return sinpif(xf);
		case COSPIF:
			return cospif(xf);
		case TANPIF:
			return tanpif(xf);
		case ASINPIF:
			return asinpif(xf);
		case ACOSPIF:
			return acospif(xf);
		case ATANPIF:
			return atanpif(xf);
		case ATAN2PIF:
			return atan2pif(xf, bf);
		case SINHF:
			return sinhf(xf);
		case COSHF:
			return coshf(xf);
		case TANHF:
			return tanhf(xf);
		case ASINHF:
			return asinhf(xf);
		case ACOSHF:
			return acoshf(xf);
		case ATANHF:
			return atanhf(xf);
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
	/* Nearest: 1 ulp for double, correct rounding for float; directed: 2 and 1 ulp */
	double worst[40] = { 0 };

	for (int direction = 0; direction < 4; ++direction) {
		if (fesetround(direction) != 0)
			continue;
		for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
			bool single = vectors[i].function >= SINF;
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

static void check_trigonometric(void)
{
	/* F.10.1.6, F.10.1.5, F.10.1.7: odd functions keep the sign of zero; infinities are domain errors */
	EXPECT(sin(-0.0) == 0 && signbit(sin(-0.0)) && tan(-0.0) == 0 && signbit(tan(-0.0)), 0);
	EXPECT(cos(-0.0) == 1.0 && cos(0.0) == 1.0, 0);
	EXPECT(isnan(sin(INFINITY)) && isnan(cos(-INFINITY)) && isnan(tan(INFINITY)), INVALID);
	EXPECT(isnan(sin(NAN)) && isnan(cos(NAN)) && isnan(tanf(NAN)), 0);
	EXPECT(libm_exact("sin", sin(0x1p-1070), 0x1p-1070), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("sin", sin(0x1p-30), nextafter(0x1p-30, 0.0)), INEXACT);
	IN_DIRECTION(1, libm_directed("tan", tan(0x1p-30), nextafter(0x1p-30, 1.0)), INEXACT);
	IN_DIRECTION(2, libm_directed("cos", cos(0x1p-30), nextafter(1.0, 0.0)), INEXACT);
	EXPECT(libm_exact("cos", cos(0x1p-1074), 1.0), INEXACT);
	IN_DIRECTION(1, libm_directed("sin", sinf(0x1p-140f), 0x1p-140f), UNDERFLOW | INEXACT);
	IN_DIRECTION(2, libm_directed("sin", sinf(0x1p-140f), nextafterf(0x1p-140f, 0.0f)), UNDERFLOW | INEXACT);
	CHECK(libm_exact("sin", sin(0x1.921fb54442d18p+0), 1.0) && libm_exact("cos", cos(0x1.921fb54442d18p+0), 0x1.1a62633145c07p-54));
	CHECK(libm_exact("sin", sin(0x1.921fb54442d18p+1), 0x1.1a62633145c07p-53) && libm_exact("tan", tan(0x1.921fb54442d18p-1), 0x1.fffffffffffffp-1));
	CHECK(libm_exact("sin", sin(0x1p1023), 0x1.205248cbdb760p-1) && libm_exact("cos", cos(0x1p1023), -0x1.a719f26c232bfp-1));

	/* F.10.1.2, F.10.1.1, F.10.1.3, F.10.1.4 */
	EXPECT(asin(-0.0) == 0 && signbit(asin(-0.0)) && atan(-0.0) == 0 && signbit(atan(-0.0)), 0);
	EXPECT(acos(1.0) == 0 && !signbit(acos(1.0)), 0);
	IN_DIRECTION(2, acos(1.0) == 0 && !signbit(acos(1.0)), 0);
	EXPECT(isnan(asin(1.5)) && isnan(acos(-1.0000000000000002)) && isnan(asinf(-INFINITY)), INVALID);
	CHECK(libm_exact("asin", asin(1.0), 0x1.921fb54442d18p+0) && libm_exact("acos", acos(-1.0), 0x1.921fb54442d18p+1) && libm_exact("acos", acos(0.0), 0x1.921fb54442d18p+0));
	CHECK(atan(INFINITY) == 0x1.921fb54442d18p+0 && libm_exact("atan", atan(-0x1p100), -0x1.921fb54442d18p+0) && libm_exact("atan", atan(1.0), 0x1.921fb54442d18p-1));
	EXPECT(libm_exact("asin", asin(0x1p-1070), 0x1p-1070) && libm_exact("atan", atan(-0x1p-1070), -0x1p-1070), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("asin", asin(0x1p-30), nextafter(0x1p-30, 1.0)), INEXACT);
	IN_DIRECTION(1, libm_directed("atan", atan(0x1p-30), 0x1p-30), INEXACT);
	EXPECT(atan2(0.0, -0.0) == 0x1.921fb54442d18p+1 && atan2(-0.0, -1.0) == -0x1.921fb54442d18p+1, INEXACT);
	EXPECT(atan2(-0.0, 0.0) == 0 && signbit(atan2(-0.0, 0.0)) && atan2(0.0, 5.0) == 0 && !signbit(atan2(0.0, 5.0)), 0);
	EXPECT(atan2(1.0, 0.0) == 0x1.921fb54442d18p+0 && atan2(-INFINITY, 2.0) == -0x1.921fb54442d18p+0, INEXACT);
	EXPECT(atan2(INFINITY, INFINITY) == 0x1.921fb54442d18p-1 && atan2(-INFINITY, -INFINITY) == -0x1.2d97c7f3321d2p+1, INEXACT);
	EXPECT(atan2(1.0, INFINITY) == 0 && atan2(-1.0, INFINITY) == 0 && signbit(atan2(-1.0, INFINITY)), 0);
	EXPECT(isnan(atan2(NAN, 1.0)) && isnan(atan2(1.0, NAN)), 0);
	EXPECT(atan2(0x1p-1000, 0x1p100) == 0 && atan2(-0x1p-1022, DBL_MAX) == 0 && signbit(atan2(-0x1p-1022, DBL_MAX)), UNDERFLOW | INEXACT);
	EXPECT(libm_exact("atan2", atan2(0x1p-1060, 1.0), 0x1p-1060), UNDERFLOW | INEXACT);
	IN_DIRECTION(3, libm_directed("atan2", atan2(0x1p-1060, 1.0), 0x1p-1060 - 0x1p-1074), UNDERFLOW | INEXACT);
	CHECK(libm_exact("atan2", atan2(DBL_MAX, 0x1p-1022), 0x1.921fb54442d18p+0) && libm_exact("atan2", atan2(1.0, 1.0), 0x1.921fb54442d18p-1) && libm_exact("atan2", atan2(-1.0, -1.0), -0x1.2d97c7f3321d2p+1));
	EXPECT(libm_exactf("atan2", atan2f(0x1p-127f, 1.0f), 0x1p-127f), UNDERFLOW | INEXACT);
	IN_DIRECTION(3, libm_directed("atan2", atan2f(0x1p-127f, 1.0f), nextafterf(0x1p-127f, 0.0f)), UNDERFLOW | INEXACT);
	CHECK(libm_exact("atan2", (double)atan2l(1.0L, -1.0L), 0x1.2d97c7f3321d2p+1) && libm_exact("acos", (double)acosl(-1.0L), 0x1.921fb54442d18p+1));
}

static void check_pi_functions(void)
{
	/* C23 7.12.4.8-15 with IEEE 754 9.2.1: exact at the integers and half-integers */
	for (int n = -6; n <= 6; ++n) {
		double x = n;

		EXPECT(sinpi(x) == 0 && (signbit(sinpi(x)) != 0) == (n < 0 || signbit(x) != 0), 0);
		EXPECT(cospi(x) == ((n & 1) != 0 ? -1.0 : 1.0) && cospi(x + 0.5) == 0 && !signbit(cospi(x + 0.5)), 0);
		EXPECT(sinpi(x + 0.5) == ((n & 1) != 0 ? -1.0 : 1.0), 0);
		EXPECT(tanpi(x) == 0 && (signbit(tanpi(x)) != 0) == ((n & 1) != 0 ? n > 0 : n < 0), 0);
		EXPECT(tanpi(x + 0.5) == ((n & 1) != 0 ? -INFINITY : INFINITY), DIVBYZERO);
	}
	EXPECT(sinpi(-0.0) == 0 && signbit(sinpi(-0.0)) && tanpi(-0.0) == 0 && signbit(tanpi(-0.0)), 0);
	EXPECT(sinpi(0x1p60) == 0 && sinpi(-0x1p60) == 0 && signbit(sinpi(-0x1p60)) && cospi(0x1p60) == 1.0, 0);
	EXPECT(cospi(0x1p52 + 1) == -1.0 && tanpi(0x1p52 + 1) == 0 && signbit(tanpi(0x1p52 + 1)), 0);
	EXPECT(isnan(sinpi(INFINITY)) && isnan(cospi(-INFINITY)) && isnan(tanpi(INFINITY)), INVALID);
	EXPECT(libm_exact("sinpi", sinpi(0.25), 0x1.6a09e667f3bcdp-1) && libm_exact("cospi", cospi(0.25), 0x1.6a09e667f3bcdp-1) && libm_exact("tanpi", tanpi(0.25), 1.0), INEXACT);
	EXPECT(libm_exact("sinpi", sinpi(1.0 / 6), 0.5) || sinpi(1.0 / 6) == 0x1.fffffffffffffp-2, INEXACT);
	EXPECT(libm_exact("sinpi", sinpi(0x1p-1074), 0x1p-1073 + 0x1p-1074), UNDERFLOW | INEXACT);
	EXPECT_EXACT(libm_exact("asinpi", asinpi(1.0), 0.5) && libm_exact("asinpi", asinpi(-1.0), -0.5) && libm_exact("acospi", acospi(-1.0), 1.0) && acospi(1.0) == 0 && libm_exact("acospi", acospi(0.0), 0.5), 0);
	EXPECT_EXACT(atanpi(INFINITY) == 0.5 && libm_exact("atanpi", atanpi(-1.0), -0.25) && atanpi(-0.0) == 0 && signbit(atanpi(-0.0)), 0);
	EXPECT(isnan(asinpi(2.0)) && isnan(acospi(-1.5)), INVALID);
	EXPECT(atan2pi(0.0, -0.0) == 1.0 && atan2pi(-0.0, -2.0) == -1.0 && atan2pi(1.0, 0.0) == 0.5, 0);
	EXPECT_EXACT(libm_exact("atan2pi", atan2pi(3.0, 3.0), 0.25) && libm_exact("atan2pi", atan2pi(-3.0, -3.0), -0.75) && atan2pi(INFINITY, -INFINITY) == 0.75, 0);
	EXPECT(atan2pi(-0.0, 1.0) == 0 && signbit(atan2pi(-0.0, 1.0)), 0);
	EXPECT(libm_exact("asinpi", asinpi(0x1p-1060), 0x145fp-1074), UNDERFLOW | INEXACT);
	EXPECT(atan2pi(0x1p-1000, 0x1p30) < 0x1p-1022, UNDERFLOW | INEXACT);
	CHECK(sinpif(0.5f) == 1.0f && cospif(1.0f) == -1.0f && libm_exactf("atan2pi", atan2pif(1.0f, -1.0f), 0.75f) && libm_exact("acospi", (double)acospil(0.5L), (double)(1.0L / 3)));
}

static void check_hyperbolic(void)
{
	/* F.10.2: sinh, tanh, asinh, atanh keep the sign of zero; cosh(0) is 1 */
	EXPECT(sinh(-0.0) == 0 && signbit(sinh(-0.0)) && tanh(-0.0) == 0 && signbit(tanh(-0.0)), 0);
	EXPECT(asinh(-0.0) == 0 && signbit(asinh(-0.0)) && atanh(-0.0) == 0 && signbit(atanh(-0.0)), 0);
	EXPECT(cosh(-0.0) == 1.0 && acosh(1.0) == 0 && !signbit(acosh(1.0)), 0);
	EXPECT(sinh(-INFINITY) == -INFINITY && cosh(-INFINITY) == INFINITY && tanh(-INFINITY) == -1.0, 0);
	EXPECT(asinh(-INFINITY) == -INFINITY && acosh(INFINITY) == INFINITY, 0);
	EXPECT(sinh(711.0) == INFINITY && cosh(-711.0) == INFINITY && sinh(-1000.0) == -INFINITY, OVERFLOW | INEXACT);
	CHECK(sinh(710.4) < DBL_MAX && libm_exact("cosh", cosh(710.4), sinh(710.4)) && sinh(710.48) == INFINITY);
	EXPECT(isnan(acosh(0.5)) && isnan(acosh(-INFINITY)) && isnan(atanh(1.5)), INVALID);
	EXPECT(atanh(1.0) == INFINITY && atanh(-1.0) == -INFINITY, DIVBYZERO);
	EXPECT(libm_exact("tanh", tanh(30.0), 1.0) && libm_exact("tanh", tanh(-30.0), -1.0), INEXACT);
	CHECK(libm_exact("tanh", tanh(15.0), 0x1.ffffffffff96ap-1) && libm_exact("tanh", tanh(-15.0), -0x1.ffffffffff96ap-1));
	IN_DIRECTION(2, libm_directed("tanh", tanh(30.0), nextafter(1.0, 0.0)), INEXACT);
	IN_DIRECTION(1, libm_directed("tanh", tanh(-30.0), nextafter(-1.0, 0.0)), INEXACT);
	IN_DIRECTION(1, libm_directed("cosh", cosh(0x1p-30), nextafter(1.0, 2.0)), INEXACT);
	EXPECT(libm_exact("cosh", cosh(0x1p-1074), 1.0), INEXACT);
	EXPECT(libm_exact("sinh", sinh(0x1p-1070), 0x1p-1070) && libm_exact("tanh", tanh(0x1p-1070), 0x1p-1070), UNDERFLOW | INEXACT);
	IN_DIRECTION(1, libm_directed("sinh", sinh(0x1p-30), nextafter(0x1p-30, 1.0)), INEXACT);
	IN_DIRECTION(2, libm_directed("asinh", asinh(0x1p-30), nextafter(0x1p-30, 0.0)), INEXACT);
	IN_DIRECTION(1, libm_directed("atanh", atanh(-0x1p-30), -0x1p-30), INEXACT);
	EXPECT(libm_exactf("sinh", sinhf(0x1p-140f), 0x1p-140f) && libm_exactf("atanh", atanhf(-0x1p-140f), -0x1p-140f), UNDERFLOW | INEXACT);
	CHECK(libm_exact("asinh", asinh(DBL_MAX), 0x1.633ce8fb9f87ep+9) && libm_exact("acosh", acosh(DBL_MAX), 0x1.633ce8fb9f87ep+9) && libm_exact("atanh", atanh(0.5), 0x1.193ea7aad030bp-1));
	CHECK(libm_exactf("cosh", coshf(1.0f), 0x1.8b0756p+0f) && libm_exact("tanh", (double)tanhl(0.5L), 0x1.d9353d7568af3p-2));
}

int main(void)
{
	check_vectors();
	check_trigonometric();
	check_pi_functions();
	check_hyperbolic();

	if (failures == 0)
		printf("pass: libm_trig (%zu vectors)\n", sizeof(vectors) / sizeof(vectors[0]));

	return failures == 0 ? 0 : 1;
}
