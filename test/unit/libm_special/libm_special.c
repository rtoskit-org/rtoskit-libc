/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_special.c
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
 * libm part 4 (P1.16, docs/research/libm-special.md): erf, erfc, lgamma and
 * tgamma. Reference vectors (600-bit mpmath) bound the error as for the
 * other libm steps (D47): at most 1 ulp for double and correct rounding for
 * float to nearest, 2 and 1 ulp in the directed modes. Then the special
 * cases of Annex F.10.5, the exact factorials and the overflow and
 * underflow boundaries.
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
	ERF,
	ERFC,
	LGAMMA,
	TGAMMA,
	ERFF,
	ERFCF,
	LGAMMAF,
	TGAMMAF,
};

static const char *const names[] = { "erf", "erfc", "lgamma", "tgamma" };

static const struct {
	enum function function;
	double x;
	double hi;
	double lo;
} vectors[] = {
#include "vectors.inc"
};

static double call(enum function function, double x)
{
	float xf = (float)x;

	switch (function) {
		case ERF:
			return erf(x);
		case ERFC:
			return erfc(x);
		case LGAMMA:
			return lgamma(x);
		case TGAMMA:
			return tgamma(x);
		case ERFF:
			return erff(xf);
		case ERFCF:
			return erfcf(xf);
		case LGAMMAF:
			return lgammaf(xf);
		case TGAMMAF:
			return tgammaf(xf);
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
	double worst[8] = { 0 };

	for (int direction = 0; direction < 4; ++direction) {
		if (fesetround(direction) != 0)
			continue;
		for (size_t i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
			bool single = vectors[i].function >= ERFF;
			double got = call(vectors[i].function, vectors[i].x);
			double error;

			fesetround(0);
			error = ulp_error(got, vectors[i].hi, vectors[i].lo, single);
			if (!libm_vector_within(names[vectors[i].function % 4], single, direction, error, got, vectors[i].hi)) {
				char message[200];

				snprintf(message, sizeof(message), "%s%s(%a) direction %d = %a, want %a + %a: %g ulp", names[vectors[i].function % 4], single ? "f" : "", vectors[i].x, direction, got, vectors[i].hi, vectors[i].lo, error);
				fail(__LINE__, message);
			}
			if (direction == 0 && error > worst[vectors[i].function])
				worst[vectors[i].function] = error;
			fesetround(direction);
		}
	}
	fesetround(0);

	for (int i = 0; i < 8; ++i)
		printf("%s%s: %.3f ulp\n", names[i % 4], i >= 4 ? "f" : "", worst[i]);
}

static void check_special_cases(void)
{
	/* F.10.5.1, F.10.5.2: erf is odd, erf(+-inf) is +-1, erfc(-inf) is 2 and erfc(+inf) +0 */
	EXPECT(erf(-0.0) == 0 && signbit(erf(-0.0)) && erf(INFINITY) == 1.0 && erf(-INFINITY) == -1.0, 0);
	EXPECT(erfc(INFINITY) == 0 && !signbit(erfc(INFINITY)) && erfc(-INFINITY) == 2.0 && erfc(0.0) == 1.0, 0);
	EXPECT(isnan(erf(NAN)) && isnan(erfc(NAN)) && isnan(lgamma(NAN)) && isnan(tgamma(NAN)), 0);
	EXPECT(libm_exact("erf", erf(0x1p-1070), 0x1.2p-1070), UNDERFLOW | INEXACT);
	EXPECT(libm_exact("erf", erf(10.0), 1.0) && libm_exact("erf", erf(-10.0), -1.0) && erfc(-10.0) == 2.0, INEXACT);
	IN_DIRECTION(2, libm_directed("erf", erf(10.0), nextafter(1.0, 0.0)), INEXACT);
	IN_DIRECTION(2, erfc(-10.0) == nextafter(2.0, 0.0), INEXACT);
	EXPECT(erfc(30.0) == 0, UNDERFLOW | INEXACT);
	IN_DIRECTION(1, erfc(30.0) == 0x1p-1074, UNDERFLOW | INEXACT);
	EXPECT(erfc(27.0) < 0x1p-1022 && erfc(27.0) > 0, UNDERFLOW | INEXACT);
	CHECK(erf(0.5) == 0x1.0a7ef5c18edd2p-1 && erfc(1.0) == 0x1.4226162fbddd5p-3 && erfc(5.0) == 0x1.b0c1a759f7739p-40);

	/* F.10.5.3: lgamma(1) = lgamma(2) = +0, poles are +inf with divide-by-zero, lgamma(+-inf) = +inf */
	EXPECT(lgamma(1.0) == 0 && !signbit(lgamma(1.0)) && lgamma(2.0) == 0 && !signbit(lgamma(2.0)), 0);
	EXPECT(lgamma(0.0) == INFINITY && lgamma(-0.0) == INFINITY && lgamma(-3.0) == INFINITY, DIVBYZERO);
	EXPECT(lgamma(INFINITY) == INFINITY && lgamma(-INFINITY) == INFINITY, 0);
	EXPECT(lgamma(0x1p1023) == INFINITY, OVERFLOW | INEXACT);
	CHECK(lgamma(0.5) == 0x1.250d048e7a1bdp-1 && lgamma(-0.5) == 0x1.43f89a3f0edd6p+0 && lgamma(1e300) == 0x1.017f38e7a1ab5p+1006);
	CHECK(lgamma(1.0 + 0x1p-40) == -0x1.2788cfc6f9bc7p-41 && lgamma(2.0 - 0x1p-40) == -0x1.b0ee607207f2bp-42);

	/* F.10.5.4: tgamma(+-0) is +-inf with divide-by-zero; negative integers and -inf are domain errors */
	EXPECT(tgamma(0.0) == INFINITY && tgamma(-0.0) == -INFINITY, DIVBYZERO);
	EXPECT(isnan(tgamma(-1.0)) && isnan(tgamma(-INFINITY)) && isnan(tgammaf(-2.0f)), INVALID);
	EXPECT(tgamma(INFINITY) == INFINITY, 0);
	EXPECT(tgamma(172.0) == INFINITY && tgamma(171.7) == INFINITY, OVERFLOW | INEXACT);
	EXPECT(tgamma(-190.5) == 0 && signbit(tgamma(-190.5)) && !signbit(tgamma(-191.5)), UNDERFLOW | INEXACT);
	for (int n = 1; n <= 23; ++n) {
		double factorial = 1.0;

		for (int k = 2; k < n; ++k)
			factorial *= k;
		EXPECT(tgamma(n) == factorial, 0);
	}
	{
		/* Beyond 22! the factorials are no longer exact: the exp(lgamma) path must round them correctly */
		static const double factorials[] = { 0x1.5e5c335f8a4cep+74, 0x1.06c52687a7b9ap+79, 0x1.9a940c33f6121p+83, 0x1.4d9849ea37eebp+88, 0x1.19787e5d9f316p+93, 0x1.ec92dd23d6967p+97, 0x1.be6518687a785p+102 };

		for (int n = 24; n <= 30; ++n)
			CHECK(tgamma(n) == factorials[n - 24]);
	}
	CHECK(tgamma(0.5) == 0x1.c5bf891b4ef6bp+0 && tgamma(-0.5) == -0x1.c5bf891b4ef6bp+1 && tgamma(171.5) == 0x1.0e1863dcad789p+1023);
	CHECK(tgamma(0x1p-60) == 0x1p60 && tgamma(-1.5) == 0x1.2e7fb0bcdf4f2p+1);
	CHECK(erff(0.5f) == 0x1.0a7ef6p-1f && tgammaf(5.0f) == 24.0f && lgammal(3.0L) == 0x1.62e42fefa39efp-1L && erfcl(0.0L) == 1.0L);
}

int main(void)
{
	check_vectors();
	check_special_cases();

	if (failures == 0)
		printf("pass: libm_special (%zu vectors)\n", sizeof(vectors) / sizeof(vectors[0]));

	return failures == 0 ? 0 : 1;
}
