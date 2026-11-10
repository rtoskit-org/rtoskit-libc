/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * float_accuracy.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "float_accuracy.h"

/*
 * The float transcendental functions against the double functions on the
 * widened argument (good to about 2^-53, far below a float ulp): the error of
 * each result in float ulps of the exact value must stay below 0.52 (faithful
 * with margin, PLAN D57, D62), and at most one result in ten thousand (plus
 * one, for the small samples of the slower boards) may differ from the
 * correctly rounded one (the reference's own double rounding included). The
 * special values must match exactly. On
 * x86_64 the test links the float-float layer of the single-precision cores
 * (libc/math/sp-nearest) and runs FLOAT_ACCURACY_SAMPLES arguments per
 * function natively; on the Arm boards it measures the library as built.
 */

static int failures;

struct unary {
	const char *name;
	float (*function)(float);
	double (*reference)(double);
	float low;
	float high;
	/* Every float in [sweep_low, sweep_high) is checked where the samples are many: the subnormal results */
	float sweep_low;
	float sweep_high;
};

static const struct unary unary[] = {
	{ "expf", expf, exp, -104.0f, 89.0f, -104.0f, -87.0f },
	{ "exp2f", exp2f, exp2, -151.0f, 128.0f, -151.0f, -126.0f },
	{ "exp10f", exp10f, exp10, -46.0f, 39.0f, -46.0f, -37.5f },
	{ "expm1f", expm1f, expm1, -18.0f, 89.0f },
	{ "exp2m1f", exp2m1f, exp2m1, -26.0f, 128.0f },
	{ "exp10m1f", exp10m1f, exp10m1, -8.0f, 39.0f },
	/* The logarithms sweep every float near 1, where only the relative accuracy of log(1 + f) matters */
	{ "logf", logf, log, 0x1p-149f, FLT_MAX, 0x1.fcp-1f, 0x1.02p0f },
	{ "log2f", log2f, log2, 0x1p-149f, FLT_MAX, 0x1.fcp-1f, 0x1.02p0f },
	{ "log10f", log10f, log10, 0x1p-149f, FLT_MAX, 0x1.fcp-1f, 0x1.02p0f },
	/* The p1 forms sweep half a binade on either side of their series threshold 2^-6 */
	{ "log1pf", log1pf, log1p, -0x1.fffffep-1f, FLT_MAX, 0x1.8p-7f, 0x1.8p-6f },
	{ "logp1f", logp1f, logp1, -0x1.fffffep-1f, FLT_MAX, -0x1.8p-6f, -0x1.8p-7f },
	{ "log2p1f", log2p1f, log2p1, -0x1.fffffep-1f, FLT_MAX, 0x1.8p-7f, 0x1.8p-6f },
	{ "log10p1f", log10p1f, log10p1, -0x1.fffffep-1f, FLT_MAX, -0x1.8p-6f, -0x1.8p-7f },
	/* The trigonometric functions over every binade, sweeping every float near a zero or a pole */
	{ "sinf", sinf, sin, -FLT_MAX, FLT_MAX, 0x1.9p1f, 0x1.94p1f },
	{ "cosf", cosf, cos, -FLT_MAX, FLT_MAX, 0x1.9p0f, 0x1.94p0f },
	{ "tanf", tanf, tan, -FLT_MAX, FLT_MAX, 0x1.9p0f, 0x1.94p0f },
	{ "sinpif", sinpif, sinpi, -FLT_MAX, FLT_MAX, 0x1.fcp-1f, 0x1.02p0f },
	{ "cospif", cospif, cospi, -FLT_MAX, FLT_MAX, 0x1.fcp-2f, 0x1.02p-1f },
	{ "tanpif", tanpif, tanpi, -FLT_MAX, FLT_MAX, 0x1.fcp-2f, 0x1.02p-1f },
	/* The inverse functions, sweeping where the table and the octant swap meet (around 1) or where sqrt(1 - x^2) cancels */
	{ "atanf", atanf, atan, -FLT_MAX, FLT_MAX, 0x1.f8p-1f, 0x1.04p0f },
	{ "asinf", asinf, asin, -1.0f, 1.0f, 0x1.f8p-1f, 1.0f },
	{ "acosf", acosf, acos, -1.0f, 1.0f, -1.0f, -0x1.f8p-1f },
	{ "atanpif", atanpif, atanpi, -FLT_MAX, FLT_MAX, 0x1.f8p-1f, 0x1.04p0f },
	{ "asinpif", asinpif, asinpi, -1.0f, 1.0f, 0x1.f8p-1f, 1.0f },
	{ "acospif", acospif, acospi, -1.0f, 1.0f, -1.0f, -0x1.f8p-1f },
	/* The hyperbolic functions, sweeping around their series thresholds or the cancellation near 1 */
	{ "sinhf", sinhf, sinh, -90.0f, 90.0f, 0x1.cp-3f, 0x1.2p-2f },
	{ "coshf", coshf, cosh, -90.0f, 90.0f, 0x1.cp-3f, 0x1.2p-2f },
	{ "tanhf", tanhf, tanh, -11.0f, 11.0f, 0x1.cp-4f, 0x1.2p-3f },
	{ "asinhf", asinhf, asinh, -FLT_MAX, FLT_MAX, 0x1.cp-4f, 0x1.2p-3f },
	{ "acoshf", acoshf, acosh, 1.0f, FLT_MAX, 1.0f, 0x1.02p0f },
	{ "atanhf", atanhf, atanh, -1.0f, 1.0f, 0x1.cp-4f, 0x1.2p-3f },
	/* The error functions, sweeping where erff becomes +-1 and where erfcf turns subnormal */
	{ "erff", erff, erf, -5.0f, 5.0f, 0x1.fp1f, 0x1.f8p1f },
	{ "erfcf", erfcf, erfc, -5.0f, 11.0f, 0x1.28p3f, 0x1.48p3f },
	/* The gamma functions, sweeping both zeros of lgammaf in (-3, -2) and the overflow of tgammaf */
	{ "lgammaf", lgammaf, lgamma, -FLT_MAX, FLT_MAX, -0x1.6p1f, -0x1.38p1f },
	{ "tgammaf", tgammaf, tgamma, -60.0f, 36.0f, 0x1.18p5f, 0x1.2p5f },
	/* The roots sweep [1, 2), every significand once */
	{ "sqrtf", sqrtf, sqrt, 0.0f, FLT_MAX, 1.0f, 2.0f },
	{ "rsqrtf", rsqrtf, rsqrt, 0.0f, FLT_MAX, 1.0f, 2.0f },
	{ "cbrtf", cbrtf, cbrt, -FLT_MAX, FLT_MAX, 1.0f, 2.0f },
};

/* f(x, y): kind 0 picks y so that x^y stays in range, kind 1 picks pairs of any and of close magnitudes, kind 2 as 1 with both signs */
struct binary {
	const char *name;
	float (*function)(float, float);
	double (*reference)(double, double);
	int kind;
};

static const struct binary binary[] = {
	{ "powf", powf, pow, 0 },
	{ "powrf", powrf, powr, 0 },
	{ "hypotf", hypotf, hypot, 1 },
	{ "atan2f", atan2f, atan2, 2 },
	{ "atan2pif", atan2pif, atan2pi, 2 },
};

/* f(x, n): kind 0 pown, 1 rootn, 2 compoundn */
struct integer {
	const char *name;
	float (*function)(float, long long);
	double (*reference)(double, long long);
	int kind;
};

static const struct integer integer[] = {
	{ "pownf", pownf, pown, 0 },
	{ "rootnf", rootnf, rootn, 1 },
	{ "compoundnf", compoundnf, compoundn, 2 },
};

static uint32_t next(uint32_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;
	return *state;
}

static float from_bits(uint32_t bits)
{
	float x;

	memcpy(&x, &bits, sizeof(x));
	return x;
}

static uint32_t bits_of(float x)
{
	uint32_t bits;

	memcpy(&bits, &x, sizeof(bits));
	return bits;
}

/* |f - exact| in ulps of the exact value's binade (subnormal ulp 2^-149); a result past FLT_MAX counts as 2^128 */
static double ulp_error(float f, double exact)
{
	double value = isinf(f) ? copysign(0x1p128, f) : f;
	double magnitude = fabs(exact);
	int exponent;

	if (isnan(exact))
		return isnan(f) ? 0.0 : INFINITY;
	if (isinf(exact))
		return f == exact ? 0.0 : INFINITY;
	/* The sign must match, zeros included (a result rounding to zero keeps the sign of the exact value) */
	if (isnan(f) || !signbit(f) != !signbit(exact))
		return INFINITY;
	/* Past FLT_MAX a faithful result is FLT_MAX or infinity */
	if (magnitude > FLT_MAX)
		return (isinf(f) || fabsf(f) == FLT_MAX) && !signbit(f) == !signbit(exact) ? 0.0 : INFINITY;
	frexp(magnitude, &exponent);
	if (exponent < -125)
		exponent = -125;

	return fabs(value - exact) / ldexp(1.0, exponent - 24);
}

/* Random floats in [low, high]: random bit patterns (every binade) and uniform values, alternately */
static float sample(uint32_t *state, float low, float high, int i)
{
	float x;

	if ((i & 1) != 0) {
		float u = (float)(next(state) >> 8) * 0x1p-24f;

		/* Interpolated without forming high - low, which overflows for the full range */
		return low * (1.0f - u) + high * u;
	}
	do {
		x = from_bits(next(state));
	} while (!(x >= low && x <= high));

	return x;
}

static void check_specials(const struct unary *test)
{
	static const float specials[] = { 0.0f, -0.0f, INFINITY, -INFINITY, NAN, 1.0f, -1.0f, 0x1p-149f, -0x1p-149f, 0x1p-126f, -0x1p-126f, 0x1p-30f, -0x1p-30f };

	for (size_t i = 0; i < sizeof(specials) / sizeof(specials[0]); ++i) {
		float x = specials[i];
		float got = test->function(x);
		float expected = (float)test->reference(x);
		char message[128];

		if (isnan(got) && isnan(expected))
			continue;
		if (bits_of(got) == bits_of(expected) || ulp_error(got, test->reference(x)) < float_accuracy_tolerance(test->name, test->reference(x)))
			continue;
		snprintf(message, sizeof(message), "fail: %s(%a) = %a, expected %a\n", test->name, (double)x, (double)got, (double)expected);
		fputs(message, stdout);
		++failures;
	}
}

static void check_unary(const struct unary *test)
{
	uint32_t state = 0x2545f491;
	double worst = 0;
	float worst_x = 0;
	long misrounded = 0;
	char message[160];

	long count = FLOAT_ACCURACY_SAMPLES;
	long sweep = 0;
	float swept = test->sweep_low;

	check_specials(test);
	/* Where the platform runs millions of arguments, the sweep adds every float of the subnormal range */
	if (FLOAT_ACCURACY_SAMPLES >= 0x200000 && test->sweep_low < test->sweep_high)
		sweep = 1;
	for (long i = 0; i < count || (sweep != 0 && swept < test->sweep_high); ++i) {
		float x;
		double exact;

		if (i < count) {
			x = sample(&state, test->low, test->high, (int)i);
		} else {
			x = swept;
			swept = nextafterf(swept, INFINITY);
			++sweep;
		}
		exact = test->reference(x);
		float got = test->function(x);
		double error = ulp_error(got, exact);

		if (got != (float)exact && !(isnan(got) && isnan(exact)))
			++misrounded;
		if (error >= float_accuracy_tolerance(test->name, exact) && failures < 20) {
			snprintf(message, sizeof(message), "fail: %s(%a) = %a, exact %a\n", test->name, (double)x, (double)got, exact);
			fputs(message, stdout);
			++failures;
		}
		if (error > worst) {
			worst = error;
			worst_x = x;
		}
	}

	count += sweep;
	snprintf(message, sizeof(message), "%s: max %.3f ulp at %a, %ld of %ld not correctly rounded\n", test->name, worst, (double)worst_x, misrounded, count);
	fputs(message, stdout);
	/* Faithful everywhere, and correctly rounded but for rare near-halfway cases (expect.c, fast/expect.c) */
	if (!float_accuracy_within(test->name, worst, misrounded, count))
		++failures;
}

/* Error statistics over the samples of one function */
struct statistics {
	double worst;
	float worst_x;
	float worst_y;
	long misrounded;
	long count;
};

static void record(struct statistics *statistics, const char *name, float x, float y, float got, double exact)
{
	double error = ulp_error(got, exact);
	char message[192];

	++statistics->count;
	if (got != (float)exact && !(isnan(got) && isnan(exact)))
		++statistics->misrounded;
	if (error >= float_accuracy_tolerance(name, exact) && failures < 20) {
		snprintf(message, sizeof(message), "fail: %s(%a, %a) = %a, exact %a\n", name, (double)x, (double)y, (double)got, exact);
		fputs(message, stdout);
		++failures;
	}
	if (error > statistics->worst) {
		statistics->worst = error;
		statistics->worst_x = x;
		statistics->worst_y = y;
	}
}

static void report(const struct statistics *statistics, const char *name)
{
	char message[192];

	snprintf(message, sizeof(message), "%s: max %.3f ulp at (%a, %a), %ld of %ld not correctly rounded\n", name, statistics->worst, (double)statistics->worst_x, (double)statistics->worst_y, statistics->misrounded, statistics->count);
	fputs(message, stdout);
	if (!float_accuracy_within(name, statistics->worst, statistics->misrounded, statistics->count))
		++failures;
}

/* A positive finite float of any binade */
static float positive(uint32_t *state)
{
	float x;

	do {
		x = from_bits(next(state) & 0x7fffffff);
	} while (!(x > 0 && isfinite(x)));

	return x;
}

/* The special values of every pair must give exactly what the double function gives (C23 F.10.4) */
static const float binary_specials[] = { 0.0f, -0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, -2.0f, 3.0f, -3.0f, 0x1p-149f, INFINITY, -INFINITY, NAN };

static void check_binary_specials(const struct binary *test)
{
	char message[160];

	for (size_t i = 0; i < sizeof(binary_specials) / sizeof(binary_specials[0]); ++i) {
		for (size_t j = 0; j < sizeof(binary_specials) / sizeof(binary_specials[0]); ++j) {
			float x = binary_specials[i];
			float y = binary_specials[j];
			float got = test->function(x, y);
			double exact = test->reference(x, y);

			if ((isnan(got) && isnan(exact)) || ulp_error(got, exact) < float_accuracy_tolerance(test->name, exact))
				continue;
			snprintf(message, sizeof(message), "fail: %s(%a, %a) = %a, expected %a\n", test->name, (double)x, (double)y, (double)got, exact);
			fputs(message, stdout);
			++failures;
		}
	}
}

static void check_binary(const struct binary *test)
{
	uint32_t state = 0x7f4a7c15;
	struct statistics statistics = { 0 };

	check_binary_specials(test);
	for (int i = 0; i < FLOAT_ACCURACY_SAMPLES; ++i) {
		float x = positive(&state);
		float y;

		if (test->kind == 0) {
			/* y such that x^y lands between 2^-155 and 2^132, or y of any binade */
			double z = -155.0 + 287.0 * (double)(next(&state) >> 8) * 0x1p-24;

			y = (i & 3) != 0 && log2(x) != 0 ? (float)(z / log2(x)) : from_bits(next(&state) & 0x83ffffff);
			if (test->function == powf && (i & 7) == 0) {
				/* A negative x with an integer y */
				x = -x;
				y = truncf(y);
			}
		} else {
			y = (i & 1) != 0 ? x * (0.5f + (float)(next(&state) >> 8) * 0x1p-23f) : positive(&state);
			if ((i & 2) != 0)
				x = -x;
			if (test->kind == 2 && (i & 4) != 0)
				y = -y;
		}
		record(&statistics, test->name, x, y, test->function(x, y), test->reference(x, y));
	}
	report(&statistics, test->name);
}

static void check_integer(const struct integer *test)
{
	uint32_t state = 0x1b873593;
	struct statistics statistics = { 0 };
	static const long long specials[] = { 0, 1, -1, 2, -2, 3, -3, 0x7fffffffffffffffLL, -0x7fffffffffffffffLL - 1 };
	char message[160];

	for (size_t i = 0; i < sizeof(binary_specials) / sizeof(binary_specials[0]); ++i) {
		for (size_t j = 0; j < sizeof(specials) / sizeof(specials[0]); ++j) {
			float x = binary_specials[i];
			float got = test->function(x, specials[j]);
			double exact = test->reference(x, specials[j]);

			if ((isnan(got) && isnan(exact)) || ulp_error(got, exact) < float_accuracy_tolerance(test->name, exact))
				continue;
			snprintf(message, sizeof(message), "fail: %s(%a, %lld) = %a, expected %a\n", test->name, (double)x, specials[j], (double)got, exact);
			fputs(message, stdout);
			++failures;
		}
	}

	for (int i = 0; i < FLOAT_ACCURACY_SAMPLES; ++i) {
		float x = positive(&state);
		long long n;

		if (test->kind == 2) {
			/* 1 + x in (0, inf): x in (-1, inf) */
			if ((i & 1) != 0)
				x = -(float)(next(&state) >> 8) * 0x1p-24f;
		} else if ((i & 3) == 0) {
			x = -x;
		}

		if ((i & 1) != 0) {
			n = (long long)(next(&state) % 101) - 50;
		} else {
			/* n large where the result stays in range: n = z / log2|x| (log2(1 + x) for compoundn) */
			double z = -155.0 + 287.0 * (double)(next(&state) >> 8) * 0x1p-24;
			double log = test->kind == 2 ? log2p1(x) : log2(fabs(x));
			double quotient = test->kind == 1 ? log / z : z / log;

			n = log != 0 && fabs(quotient) < 0x1p62 ? (long long)quotient : (long long)(next(&state) % 101) - 50;
		}
		/* Odd roots of negative values, so that most results are numbers */
		if (test->kind == 1 && x < 0)
			n |= 1;
		record(&statistics, test->name, x, (float)n, test->function(x, n), test->reference(x, n));
	}
	report(&statistics, test->name);
}

int main(void)
{
	for (size_t i = 0; i < sizeof(unary) / sizeof(unary[0]); ++i)
		check_unary(&unary[i]);
	for (size_t i = 0; i < sizeof(binary) / sizeof(binary[0]); ++i)
		check_binary(&binary[i]);
	for (size_t i = 0; i < sizeof(integer) / sizeof(integer[0]); ++i)
		check_integer(&integer[i]);

	fputs(failures == 0 ? "float_accuracy: pass\n" : "float_accuracy: FAIL\n", stdout);
	return failures == 0 ? 0 : 1;
}
