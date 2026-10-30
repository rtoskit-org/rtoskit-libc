/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_vectors.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Every <math.h> function in float, double and long double against vectors
 * a correctly rounded oracle generated (tools/libm-vectors, MPFR; issue #95,
 * docs/research/libm-vectors.md). The bounds are the library's documented
 * contracts, each no looser than libc-test's (1.5 ulp to nearest, 3 ulp in
 * the directed modes):
 * - double and long double: within 1 ulp of the exact result (D47, D48);
 * - float: correctly rounded where every floating type is the FPU's or the
 *   double path's (the cores where <fenv.h> is complete, D98), and within
 *   1 ulp on the float-float layer of the other cores (D57);
 * - exact functions (rounding to integer, fmod, scaling, ...): exact;
 * - in the directed rounding modes, where the core has them: float within
 *   1 ulp and double within 3 ulp of the correctly rounded result (Q36
 *   records the float cases that are not correctly rounded).
 * The largest error of each function is printed where it exceeds half an ulp
 * to nearest, so the measured accuracy stays visible.
 * Where <fenv.h> has exceptions, invalid, divide-by-zero and overflow must
 * be exactly those C23 F.10 specifies, and a deserved underflow must be
 * raised. F.10 leaves inexact and undeserved underflow unspecified, so they
 * are not checked; nor is underflow where tininess before and after
 * rounding disagree, or invalid where the standard says "may".
 */

#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libm_vectors.h"

/* The exceptions the vectors describe; zero where the core keeps none (D44) */
#ifdef FE_INVALID
static const int exceptions = FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW;
#else
static const int exceptions = 0;
#endif

/* The directions beyond to-nearest, where the core has them (C23 7.6p8) */
#ifdef FE_UPWARD
static const int directions[4] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
static const int direction_count = 4;
static const bool float_is_correctly_rounded = true;
#else
static const int directions[4] = { FE_TONEAREST };
static const int direction_count = 1;
static const bool float_is_correctly_rounded = false;
#endif

static unsigned long checked;
static unsigned long failures;

/* The largest error seen for the current function: to nearest against the exact value, directed against the correctly rounded one */
static double worst_nearest;
static double worst_directed;

/* One ulp at |x| in a format of the given precision (subnormals have the fixed ulp of the smallest normal binade) */
static double ulp(double x, int precision)
{
	int min_exponent = precision == 24 ? -125 : -1021;
	int exponent;

	if (x == 0.0 || !isfinite(x))
		x = ldexp(1.0, min_exponent - 1);
	frexp(fabs(x), &exponent);
	if (exponent < min_exponent)
		exponent = min_exponent;
	return ldexp(1.0, exponent - precision);
}

static bool same_bits(double got, double want, bool zero_sign_any)
{
	if (isnan(want))
		return isnan(got);
	if (got != want)
		return false;
	return zero_sign_any || want != 0.0 || signbit(got) == signbit(want);
}

/* The result of one call, as double, and its integer output */
struct outcome {
	double value;
	double extra;
	long long integer;
	int raised;
};

static struct outcome call(const struct libm_function *f, const struct libm_vector *v, int precision_kind)
{
	struct outcome out = {};
	int exponent = 0;

	feclearexcept(FE_ALL_EXCEPT);
	/* precision_kind: 0 double, 1 long double, 2 float */
	switch (f->kind) {
		case LIBM_UNARY:
			if (precision_kind == 2)
				out.value = ((float (*)(float))f->pointer)((float)v->x);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double))f->pointer)(v->x);
			else
				out.value = ((double (*)(double))f->pointer)(v->x);
			break;
		case LIBM_BINARY:
		case LIBM_NEXTAFTER:
			if (precision_kind == 2)
				out.value = ((float (*)(float, float))f->pointer)((float)v->x, (float)v->y);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, long double))f->pointer)(v->x, v->y);
			else
				out.value = ((double (*)(double, double))f->pointer)(v->x, v->y);
			break;
		case LIBM_NEXTTOWARD:
			if (precision_kind == 2)
				out.value = ((float (*)(float, long double))f->pointer)((float)v->x, v->y);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, long double))f->pointer)(v->x, v->y);
			else
				out.value = ((double (*)(double, long double))f->pointer)(v->x, v->y);
			break;
		case LIBM_TERNARY:
			if (precision_kind == 2)
				out.value = ((float (*)(float, float, float))f->pointer)((float)v->x, (float)v->y, (float)v->z);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, long double, long double))f->pointer)(v->x, v->y, v->z);
			else
				out.value = ((double (*)(double, double, double))f->pointer)(v->x, v->y, v->z);
			break;
		case LIBM_SCALE:
			if (precision_kind == 2)
				out.value = ((float (*)(float, int))f->pointer)((float)v->x, (int)v->y);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, int))f->pointer)(v->x, (int)v->y);
			else
				out.value = ((double (*)(double, int))f->pointer)(v->x, (int)v->y);
			break;
		case LIBM_SCALBLN:
			if (precision_kind == 2)
				out.value = ((float (*)(float, long))f->pointer)((float)v->x, (long)v->y);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, long))f->pointer)(v->x, (long)v->y);
			else
				out.value = ((double (*)(double, long))f->pointer)(v->x, (long)v->y);
			break;
		case LIBM_TO_LONG:
			if (precision_kind == 2)
				out.integer = ((long (*)(float))f->pointer)((float)v->x);
			else if (precision_kind == 1)
				out.integer = ((long (*)(long double))f->pointer)(v->x);
			else
				out.integer = ((long (*)(double))f->pointer)(v->x);
			break;
		case LIBM_TO_LLONG:
			if (precision_kind == 2)
				out.integer = ((long long (*)(float))f->pointer)((float)v->x);
			else if (precision_kind == 1)
				out.integer = ((long long (*)(long double))f->pointer)(v->x);
			else
				out.integer = ((long long (*)(double))f->pointer)(v->x);
			break;
		case LIBM_ILOGB:
			if (precision_kind == 2)
				out.integer = ((int (*)(float))f->pointer)((float)v->x);
			else if (precision_kind == 1)
				out.integer = ((int (*)(long double))f->pointer)(v->x);
			else
				out.integer = ((int (*)(double))f->pointer)(v->x);
			break;
		case LIBM_FREXP:
			if (precision_kind == 2)
				out.value = ((float (*)(float, int *))f->pointer)((float)v->x, &exponent);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, int *))f->pointer)(v->x, &exponent);
			else
				out.value = ((double (*)(double, int *))f->pointer)(v->x, &exponent);
			out.integer = exponent;
			break;
		case LIBM_MODF:
			if (precision_kind == 2) {
				float integral;

				out.value = ((float (*)(float, float *))f->pointer)((float)v->x, &integral);
				out.extra = integral;
			} else if (precision_kind == 1) {
				long double integral;

				out.value = (double)((long double (*)(long double, long double *))f->pointer)(v->x, &integral);
				out.extra = (double)integral;
			} else {
				out.value = ((double (*)(double, double *))f->pointer)(v->x, &out.extra);
			}
			break;
		case LIBM_REMQUO:
			if (precision_kind == 2)
				out.value = ((float (*)(float, float, int *))f->pointer)((float)v->x, (float)v->y, &exponent);
			else if (precision_kind == 1)
				out.value = (double)((long double (*)(long double, long double, int *))f->pointer)(v->x, v->y, &exponent);
			else
				out.value = ((double (*)(double, double, int *))f->pointer)(v->x, v->y, &exponent);
			out.integer = exponent;
			break;
	}
	out.raised = fetestexcept(FE_ALL_EXCEPT);
	return out;
}

static void report(const struct libm_function *f, const struct libm_vector *v, int direction, const char *what, double got, double want)
{
	if (failures < 100000)
		printf("FAIL %s(%a, %a, %a) direction %d: %s: got %a, want %a\n", f->name, v->x, v->y, v->z, direction, what, got, want);
	++failures;
}

/* The flags the vector expects, against the ones the call raised */
static void check_flags([[maybe_unused]] const struct libm_function *f, [[maybe_unused]] const struct libm_vector *v, [[maybe_unused]] int direction, [[maybe_unused]] int expected, [[maybe_unused]] int raised)
{
#ifdef FE_INVALID
	if (exceptions == 0)
		return;

	int mask = exceptions & ~FE_UNDERFLOW;
	int want = 0;

	if ((expected & LIBM_INVALID_ANY) != 0)
		mask &= ~FE_INVALID;
	/*
	 * C23 F.10: an undeserved underflow is unspecified, so underflow is only
	 * required where it is deserved, and then only in to-nearest where tininess
	 * before and after rounding agree (libc-test checks no directed underflow)
	 */
	if ((expected & LIBM_UNDERFLOW) != 0 && (expected & LIBM_UNDERFLOW_ANY) == 0 && direction == 0)
		mask |= FE_UNDERFLOW;
	if ((expected & LIBM_INVALID) != 0)
		want |= FE_INVALID;
	if ((expected & LIBM_DIVBYZERO) != 0)
		want |= FE_DIVBYZERO;
	if ((expected & LIBM_OVERFLOW) != 0)
		want |= FE_OVERFLOW;
	if ((expected & LIBM_UNDERFLOW) != 0)
		want |= FE_UNDERFLOW;
	if ((raised & mask) != (want & mask))
		report(f, v, direction, "exceptions", (double)(raised & mask), (double)(want & mask));
#endif
	/* Without FE_INVALID (C23 7.6p6) the core keeps no exceptions: nothing to compare */
}

/* The value, within the bound for the function and direction */
static void check_value(const struct libm_function *f, const struct libm_vector *v, int direction, double got, bool exact)
{
	double want = v->result[direction];
	bool zero_sign_any = (v->flags[direction] & LIBM_ZERO_SIGN_ANY) != 0;

	if (isnan(want) || isinf(want) || exact || (direction == 0 && v->delta == 0.0)) {
		if (!same_bits(got, want, zero_sign_any))
			report(f, v, direction, "value", got, want);
		return;
	}
	if (!isfinite(got)) {
		report(f, v, direction, "value", got, want);
		return;
	}

	double error = (got - want) / ulp(want, f->precision);

	if (direction == 0) {
		/* The error against the exact value, in ulps */
		error -= v->delta;
		if (fabs(error) > worst_nearest)
			worst_nearest = fabs(error);
		if (f->precision == 24 && float_is_correctly_rounded ? got != want : fabs(error) >= 1.0)
			report(f, v, direction, "error beyond the bound", got, want);
	} else {
		if (fabs(error) > worst_directed)
			worst_directed = fabs(error);
		if (fabs(error) > (f->precision == 24 ? 1.0 : 3.0))
			report(f, v, direction, "directed rounding", got, want);
	}
}

/* Exact, or correctly rounded by IEEE 754 itself: the result must be the correctly rounded value in every direction */
static bool is_exact_function(const struct libm_function *f, int precision_kind)
{
	static const char *const names[] = { "ceil", "floor", "trunc", "round", "rint", "nearbyint", "fabs", "fmod", "remainder", "copysign", "fmax", "fmin", "logb", "sqrt", "fma", "fdim" };
	size_t length = strlen(f->name) - (precision_kind != 0 ? 1 : 0);

	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
		if (strlen(names[i]) == length && strncmp(f->name, names[i], length) == 0)
			return true;
	return false;
}

static void check_function(const struct libm_function *f, int precision_kind)
{
	worst_nearest = 0.0;
	worst_directed = 0.0;

	bool exact = is_exact_function(f, precision_kind) || f->kind == LIBM_SCALE || f->kind == LIBM_SCALBLN || f->kind == LIBM_NEXTAFTER || f->kind == LIBM_NEXTTOWARD || f->kind == LIBM_FREXP || f->kind == LIBM_MODF || f->kind == LIBM_REMQUO;

	for (size_t i = 0; i < f->count; ++i) {
		const struct libm_vector *v = &f->vectors[i];

		for (int d = 0; d < direction_count; ++d) {
			fesetround(directions[d]);
			struct outcome out = call(f, v, precision_kind);
			fesetround(FE_TONEAREST);
			int expected = v->flags[d];

			++checked;
			switch (f->kind) {
				case LIBM_TO_LONG:
				case LIBM_TO_LLONG: {
					/* F.10.6.5, F.10.6.7: in range the value, otherwise only invalid (the value is unspecified) */
					double limit = f->kind == LIBM_TO_LONG ? (double)LONG_MAX : (double)LLONG_MAX;
					double want = v->result[d];

					if (isfinite(want) && want >= -limit - 1.0 && want < limit + 1.0) {
						if ((double)out.integer != want)
							report(f, v, d, "value", (double)out.integer, want);
					} else {
						expected |= LIBM_INVALID;
					}
					expected |= LIBM_UNDERFLOW_ANY;
					break;
				}
				case LIBM_ILOGB: {
					double want = v->result[d];
					long long want_integer = isnan(want) ? FP_ILOGBNAN : want == -INFINITY ? FP_ILOGB0 :
																	 want == INFINITY	   ? INT_MAX :
																							 (long long)want;

					if (out.integer != want_integer)
						report(f, v, d, "value", (double)out.integer, (double)want_integer);
					break;
				}
				case LIBM_FREXP:
					check_value(f, v, d, out.value, true);
					if (isfinite(v->x) && out.integer != v->aux)
						report(f, v, d, "exponent", (double)out.integer, (double)v->aux);
					break;
				case LIBM_MODF:
					check_value(f, v, d, out.value, true);
					if (!same_bits(out.extra, v->extra, false))
						report(f, v, d, "integral part", out.extra, v->extra);
					break;
				case LIBM_REMQUO:
					check_value(f, v, d, out.value, true);
					/* C23 7.12.10.3: the sign of x/y, and the low three bits of the integral quotient (aux is sign * (8 + low bits)) */
					if (!isnan(v->result[d]) && ((out.integer < 0) != (v->aux < 0) || (llabs(out.integer) & 7) != labs(v->aux) - 8))
						report(f, v, d, "quotient", (double)out.integer, (double)v->aux);
					break;
				default:
					check_value(f, v, d, out.value, exact);
					break;
			}
			check_flags(f, v, d, expected, out.raised);
		}
	}
	if (worst_nearest > 0.5 || worst_directed > 0.0)
		printf("note %s: largest error %.3f ulp to nearest, %.0f ulp directed\n", f->name, worst_nearest, worst_directed);
}
int main(void)
{
	for (size_t i = 0; i < libm_double_function_count; ++i)
		check_function(&libm_double_functions[i], 0);
	for (size_t i = 0; i < libm_long_double_function_count; ++i)
		check_function(&libm_long_double_functions[i], 1);
	for (size_t i = 0; i < libm_float_function_count; ++i)
		check_function(&libm_float_functions[i], 2);

	printf("libm_vectors: %lu checks, %lu failures (%d rounding direction%s, exceptions %s)\n", checked, failures, direction_count, direction_count == 1 ? "" : "s", exceptions != 0 ? "checked" : "not kept by this core");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
