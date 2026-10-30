/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm-vectors.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Generates the libm conformance vectors (issue #95,
 * docs/research/libm-vectors.md): for every <math.h> function in float and
 * double, special values and deterministic pseudo-random arguments, with the
 * correctly rounded result in each of the four rounding directions, the
 * exact result's offset from the nearest one in ulps, and the IEEE 754
 * exceptions each direction raises. MPFR is the oracle: each result is
 * computed at the target precision and exponent range, then subnormalized,
 * so it is the correctly rounded value of the target format. A host program,
 * built and run by tools/scripts/libm-vectors; never part of the library.
 *
 * Usage: libm-vectors double|float > vectors-<type>.inc
 */

#include <inttypes.h>
#include <math.h>
#include <mpfr.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The flags written for each rounding direction (test/unit/libm_vectors/libm_vectors.h has the same values) */
enum {
	FLAG_INVALID = 1,
	FLAG_DIVBYZERO = 2,
	FLAG_OVERFLOW = 4,
	FLAG_UNDERFLOW = 8,
	FLAG_UNDERFLOW_ANY = 16,
	FLAG_INVALID_ANY = 32,
	FLAG_ZERO_SIGN_ANY = 64,
};

enum kind {
	UNARY, /* T f(T) through an MPFR function */
	BINARY, /* T f(T, T) */
	TERNARY, /* fma */
	SCALE, /* T f(T, int) and scalbln */
	TO_INTEGER, /* lrint, llrint, lround, llround: the integral value as T */
	ILOGB, /* int ilogb(T): the result as T */
	FREXP, /* T frexp(T, int *): aux is the exponent */
	MODF, /* T modf(T, T *): extra is the integral part */
	REMQUO, /* T remquo(T, T, int *): aux is sign * (8 + the quotient's low three bits) */
	NEXTAFTER, /* nextafter and nexttoward */
	LOGB, /* logb */
};

typedef int (*mpfr_unary)(mpfr_ptr, mpfr_srcptr, mpfr_rnd_t);
typedef int (*mpfr_binary)(mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);

struct function {
	const char *name;
	enum kind kind;
	mpfr_unary unary;
	mpfr_binary binary;
	double low; /* the interval for a third of the random arguments */
	double high;
};

/* The format being generated */
static int precision;
static mpfr_exp_t format_emin;
static mpfr_exp_t format_emax;
static bool single;

static const mpfr_rnd_t directions[4] = { MPFR_RNDN, MPFR_RNDU, MPFR_RNDD, MPFR_RNDZ };

static uint64_t random_state;

static uint64_t next_random(void)
{
	random_state = random_state * 6364136223846793005u + 1442695040888963407u;
	return random_state ^ (random_state >> 29);
}

static double random_unit(void)
{
	return (double)(next_random() >> 11) * 0x1p-53;
}

/* A value of the target format: round a double to float when generating float vectors */
static double in_format(double x)
{
	return single ? (double)(float)x : x;
}

/* An argument: any finite bit pattern, a log-uniform magnitude, or the function's own interval */
static double random_argument(const struct function *f, int i)
{
	double x;

	switch (i % 3) {
		case 0:
			if (single) {
				uint32_t bits = (uint32_t)next_random();
				float value;

				memcpy(&value, &bits, sizeof(value));
				x = isfinite(value) ? value : 1.5;
			} else {
				uint64_t bits = next_random();

				memcpy(&x, &bits, sizeof(x));
				if (!isfinite(x))
					x = 1.5;
			}
			break;
		case 1:
			x = ldexp(1.0 + random_unit(), (int)(next_random() % 61) - 30);
			if (next_random() & 1)
				x = -x;
			break;
		default:
			x = f->low + (f->high - f->low) * random_unit();
			break;
	}
	return in_format(x);
}

static double special_values_double[] = { 0.0, -0.0, INFINITY, -INFINITY, NAN, 1.0, -1.0, 0.5, -0.5, 2.0, -2.0, 0x1p-1074, -0x1p-1074, 0x0.fffffffffffffp-1022, 0x1p-1022, -0x1p-1022, 0x1.fffffffffffffp1023, -0x1.fffffffffffffp1023, 3.0, -3.0, 0x1.921fb54442d18p1, 0x1.921fb54442d18p0, 1e-300, 1e300, 0x1p52, -0x1p52 + 0.5, 709.75, -745.5, 0.75, -0.75 };
static double special_values_float[] = { 0.0, -0.0, INFINITY, -INFINITY, NAN, 1.0, -1.0, 0.5, -0.5, 2.0, -2.0, 0x1p-149, -0x1p-149, 0x0.fffffep-126, 0x1p-126, -0x1p-126, 0x1.fffffep127, -0x1.fffffep127, 3.0, -3.0, 0x1.921fb6p1, 0x1.921fb6p0, 1e-30, 1e30, 0x1p23, -0x1p23 + 0.5, 88.75, -103.5, 0.75, -0.75 };

static double *special_values;
static size_t special_count;

/* Ten of the specials for the binary functions, crossed with each other */
static const int binary_specials[] = { 0, 1, 2, 3, 4, 5, 6, 9, 11, 14 };

/* One ulp of the target format at |x| */
static double ulp(double x)
{
	int exponent;

	if (x == 0.0 || !isfinite(x))
		return ldexp(1.0, (int)format_emin - 1);
	frexp(fabs(x), &exponent);
	/* Subnormals share the ulp of the smallest normal binade, whose frexp exponent is emin - 1 + precision */
	if (exponent < (int)format_emin - 1 + precision)
		exponent = (int)format_emin - 1 + precision;
	return ldexp(1.0, exponent - precision);
}

static void set_target_range(void)
{
	mpfr_set_emin(format_emin);
	mpfr_set_emax(format_emax);
}

static void set_wide_range(void)
{
	mpfr_set_emin(mpfr_get_emin_min());
	mpfr_set_emax(mpfr_get_emax_max());
}

static bool any_nan(const double *args, int count)
{
	for (int i = 0; i < count; ++i)
		if (isnan(args[i]))
			return true;
	return false;
}

/* The smallest normal of the target format */
static double smallest_normal(void)
{
	return ldexp(1.0, (int)format_emin - 1 + precision - 1);
}

struct result {
	double value[4];
	double delta;
	int flags[4];
	double extra;
	long aux;
};

/* Evaluates an MPFR function correctly rounded to the target format, with its IEEE flags */
static double evaluate(const struct function *f, const double *args, mpfr_rnd_t rnd, int *flags, double *delta, long *quotient_out)
{
	mpfr_t x, y, z, r, exact;
	int inexact;
	long quotient = 0;

	mpfr_inits2(precision, r, (mpfr_ptr)0);
	mpfr_inits2(64, x, y, z, (mpfr_ptr)0);
	mpfr_init2(exact, 400);
	set_wide_range();
	mpfr_set_d(x, args[0], MPFR_RNDN);
	mpfr_set_d(y, args[1], MPFR_RNDN);
	mpfr_set_d(z, args[2], MPFR_RNDN);

	/* The exact value, at 400 bits and MPFR's widest exponent range, for the offset and for tininess before rounding */
	mpfr_clear_flags();
	switch (f->kind) {
		case UNARY:
			f->unary(exact, x, MPFR_RNDN);
			break;
		case BINARY:
			f->binary(exact, x, y, MPFR_RNDN);
			break;
		case TERNARY:
			mpfr_fma(exact, x, y, z, MPFR_RNDN);
			break;
		case SCALE:
			mpfr_mul_2si(exact, x, (long)args[1], MPFR_RNDN);
			break;
		case REMQUO:
			mpfr_remquo(exact, &quotient, x, y, MPFR_RNDN);
			break;
		default:
			break;
	}

	/* An exact result too small even for MPFR's widest range is tiny too */
	bool exact_underflow = mpfr_underflow_p() != 0;

	set_target_range();
	mpfr_clear_flags();
	switch (f->kind) {
		case UNARY:
			inexact = f->unary(r, x, rnd);
			break;
		case BINARY:
			inexact = f->binary(r, x, y, rnd);
			break;
		case TERNARY:
			inexact = mpfr_fma(r, x, y, z, rnd);
			break;
		case SCALE:
			inexact = mpfr_mul_2si(r, x, (long)args[1], rnd);
			break;
		case REMQUO:
			inexact = mpfr_remquo(r, &quotient, x, y, rnd);
			break;
		default:
			inexact = 0;
			break;
	}
	inexact = mpfr_check_range(r, inexact, rnd);
	inexact = mpfr_subnormalize(r, inexact, rnd);

	double value = mpfr_get_d(r, MPFR_RNDN);
	int count = f->kind == TERNARY ? 3 : (f->kind == BINARY || f->kind == REMQUO) ? 2 :
																					1;

	*flags = 0;
	if (mpfr_nanflag_p() && !any_nan(args, count))
		*flags |= FLAG_INVALID;
	if (mpfr_divby0_p())
		*flags |= FLAG_DIVBYZERO;
	if (mpfr_overflow_p())
		*flags |= FLAG_OVERFLOW | FLAG_UNDERFLOW_ANY;
	/* IEEE 754 underflow: inexact and tiny; tiny before or after rounding is the implementation's choice */
	if (inexact != 0 && mpfr_number_p(exact) && (exact_underflow || !mpfr_zero_p(exact))) {
		bool tiny_before = exact_underflow || fabs(mpfr_get_d(exact, MPFR_RNDZ)) < smallest_normal();
		bool tiny_after = fabs(value) < smallest_normal();

		if (tiny_before && tiny_after)
			*flags |= FLAG_UNDERFLOW;
		else if (tiny_before != tiny_after)
			*flags |= FLAG_UNDERFLOW_ANY;
	}
	/* C23 F.10.10.1: fma(inf, 0, NaN) may or may not raise invalid */
	if (f->kind == TERNARY && isnan(args[2]))
		*flags |= FLAG_INVALID_ANY;

	if (delta != nullptr) {
		*delta = 0.0;
		if (isfinite(value) && mpfr_number_p(exact) && inexact != 0) {
			mpfr_t difference;

			mpfr_init2(difference, 400);
			set_wide_range();
			mpfr_sub_d(difference, exact, value, MPFR_RNDN);
			mpfr_div_d(difference, difference, ulp(value), MPFR_RNDN);
			*delta = mpfr_get_d(difference, MPFR_RNDN);
			mpfr_clear(difference);
		}
	}
	/* C23 7.12.10.3 fixes only the sign and the low three bits: store sign * (8 + low bits), which fits any long and keeps the sign of a zero low part */
	if (quotient_out != nullptr)
		*quotient_out = (quotient < 0 ? -1 : 1) * (8 + (labs(quotient) & 7));

	set_wide_range();
	mpfr_clears(x, y, z, r, exact, (mpfr_ptr)0);
	return value;
}

/* The exact functions and those with integer results: computed from their definitions, flags by C23 F.10 */
static void exact_function(const struct function *f, const double *args, struct result *out)
{
	double x = args[0];

	for (int d = 0; d < 4; ++d) {
		out->flags[d] = 0;
		switch (f->kind) {
			case TO_INTEGER: {
				mpfr_t a;

				mpfr_init2(a, 64);
				mpfr_set_d(a, x, MPFR_RNDN);
				if (strstr(f->name, "round") != nullptr)
					mpfr_round(a, a);
				else
					mpfr_rint(a, a, directions[d]);
				out->value[d] = mpfr_get_d(a, MPFR_RNDN);
				mpfr_clear(a);
				/* F.10.6.5, F.10.6.7: NaN, infinite or out of range raises invalid; the harness checks the range of its own long */
				if (!isfinite(x))
					out->flags[d] = FLAG_INVALID;
				break;
			}
			case ILOGB:
				out->value[d] = x == 0.0 ? -INFINITY : isnan(x) ? NAN :
												   isinf(x)		? INFINITY :
																  (double)ilogb(x);
				if (single && x != 0.0 && isfinite(x) && fabs(x) < 0x1p-126)
					out->value[d] = (double)ilogbf((float)x);
				out->flags[d] = FLAG_INVALID_ANY;
				break;
			case LOGB:
				out->value[d] = x == 0.0 ? -INFINITY : isnan(x) ? x :
												   isinf(x)		? INFINITY :
																  (double)ilogb(x);
				if (single && x != 0.0 && isfinite(x) && fabs(x) < 0x1p-126)
					out->value[d] = (double)ilogbf((float)x);
				if (x == 0.0)
					out->flags[d] = FLAG_DIVBYZERO;
				break;
			case FREXP: {
				int exponent = 0;

				out->value[d] = single ? (double)frexpf((float)x, &exponent) : frexp(x, &exponent);
				out->aux = isfinite(x) ? exponent : 0;
				break;
			}
			case MODF: {
				double integral;

				out->value[d] = modf(x, &integral);
				out->extra = integral;
				break;
			}
			case NEXTAFTER: {
				double y = args[1];
				double r = single ? (double)nextafterf((float)x, (float)y) : nextafter(x, y);

				out->value[d] = r;
				/* F.10.8.3: overflow when a finite x steps to an infinity, underflow when the result is subnormal or zero and x != y */
				if (isfinite(x) && isinf(r))
					out->flags[d] = FLAG_OVERFLOW;
				else if (x != y && !isnan(x) && !isnan(y) && fabs(r) < smallest_normal())
					out->flags[d] = FLAG_UNDERFLOW;
				break;
			}
			default:
				break;
		}
	}
	out->delta = 0.0;
}

static int mpfr_copysign_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_srcptr y, mpfr_rnd_t rnd)
{
	return mpfr_copysign(r, x, y, rnd);
}

static int mpfr_lgamma_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	int sign;

	return mpfr_lgamma(r, &sign, x, rnd);
}

static int mpfr_rint_ceil_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	return mpfr_rint_ceil(r, x, rnd), 0;
}

static int mpfr_rint_floor_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	return mpfr_rint_floor(r, x, rnd), 0;
}

static int mpfr_rint_trunc_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	return mpfr_rint_trunc(r, x, rnd), 0;
}

static int mpfr_rint_round_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	return mpfr_rint_round(r, x, rnd), 0;
}

/* rint rounds in the current direction and is inexact for a non-integer; nearbyint is the same value */
static int mpfr_rint_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_rnd_t rnd)
{
	return mpfr_rint(r, x, rnd);
}

static int mpfr_max_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_srcptr y, mpfr_rnd_t rnd)
{
	return mpfr_max(r, x, y, rnd);
}

static int mpfr_min_wrap(mpfr_ptr r, mpfr_srcptr x, mpfr_srcptr y, mpfr_rnd_t rnd)
{
	return mpfr_min(r, x, y, rnd);
}

static const struct function functions[] = {
	{ "acos", UNARY, mpfr_acos, nullptr, -1.0, 1.0 },
	{ "asin", UNARY, mpfr_asin, nullptr, -1.0, 1.0 },
	{ "atan", UNARY, mpfr_atan, nullptr, -10.0, 10.0 },
	{ "atan2", BINARY, nullptr, mpfr_atan2, -10.0, 10.0 },
	{ "cos", UNARY, mpfr_cos, nullptr, -10.0, 10.0 },
	{ "sin", UNARY, mpfr_sin, nullptr, -10.0, 10.0 },
	{ "tan", UNARY, mpfr_tan, nullptr, -10.0, 10.0 },
	{ "acosh", UNARY, mpfr_acosh, nullptr, 1.0, 100.0 },
	{ "asinh", UNARY, mpfr_asinh, nullptr, -100.0, 100.0 },
	{ "atanh", UNARY, mpfr_atanh, nullptr, -1.0, 1.0 },
	{ "cosh", UNARY, mpfr_cosh, nullptr, -20.0, 20.0 },
	{ "sinh", UNARY, mpfr_sinh, nullptr, -20.0, 20.0 },
	{ "tanh", UNARY, mpfr_tanh, nullptr, -10.0, 10.0 },
	{ "exp", UNARY, mpfr_exp, nullptr, -100.0, 100.0 },
	{ "exp2", UNARY, mpfr_exp2, nullptr, -100.0, 100.0 },
	{ "exp10", UNARY, mpfr_exp10, nullptr, -30.0, 30.0 },
	{ "expm1", UNARY, mpfr_expm1, nullptr, -2.0, 2.0 },
	{ "log", UNARY, mpfr_log, nullptr, 0.0, 10.0 },
	{ "log2", UNARY, mpfr_log2, nullptr, 0.0, 10.0 },
	{ "log10", UNARY, mpfr_log10, nullptr, 0.0, 10.0 },
	{ "log1p", UNARY, mpfr_log1p, nullptr, -1.0, 2.0 },
	{ "cbrt", UNARY, mpfr_cbrt, nullptr, -100.0, 100.0 },
	{ "sqrt", UNARY, mpfr_sqrt, nullptr, 0.0, 100.0 },
	{ "hypot", BINARY, nullptr, mpfr_hypot, -100.0, 100.0 },
	{ "pow", BINARY, nullptr, mpfr_pow, -4.0, 4.0 },
	{ "erf", UNARY, mpfr_erf, nullptr, -4.0, 4.0 },
	{ "erfc", UNARY, mpfr_erfc, nullptr, -4.0, 10.0 },
	{ "lgamma", UNARY, mpfr_lgamma_wrap, nullptr, -10.0, 20.0 },
	{ "tgamma", UNARY, mpfr_gamma, nullptr, -10.0, 20.0 },
	{ "ceil", UNARY, mpfr_rint_ceil_wrap, nullptr, -10.0, 10.0 },
	{ "floor", UNARY, mpfr_rint_floor_wrap, nullptr, -10.0, 10.0 },
	{ "trunc", UNARY, mpfr_rint_trunc_wrap, nullptr, -10.0, 10.0 },
	{ "round", UNARY, mpfr_rint_round_wrap, nullptr, -10.0, 10.0 },
	{ "rint", UNARY, mpfr_rint_wrap, nullptr, -10.0, 10.0 },
	{ "nearbyint", UNARY, mpfr_rint_wrap, nullptr, -10.0, 10.0 },
	{ "fabs", UNARY, mpfr_abs, nullptr, -10.0, 10.0 },
	{ "fmod", BINARY, nullptr, mpfr_fmod, -10.0, 10.0 },
	{ "remainder", BINARY, nullptr, mpfr_remainder, -10.0, 10.0 },
	{ "remquo", REMQUO, nullptr, nullptr, -10.0, 10.0 },
	{ "copysign", BINARY, nullptr, mpfr_copysign_wrap, -10.0, 10.0 },
	{ "fdim", BINARY, nullptr, mpfr_dim, -10.0, 10.0 },
	{ "fmax", BINARY, nullptr, mpfr_max_wrap, -10.0, 10.0 },
	{ "fmin", BINARY, nullptr, mpfr_min_wrap, -10.0, 10.0 },
	{ "fma", TERNARY, nullptr, nullptr, -10.0, 10.0 },
	{ "ldexp", SCALE, nullptr, nullptr, -10.0, 10.0 },
	{ "scalbn", SCALE, nullptr, nullptr, -10.0, 10.0 },
	{ "scalbln", SCALE, nullptr, nullptr, -10.0, 10.0 },
	{ "lrint", TO_INTEGER, nullptr, nullptr, -1e6, 1e6 },
	{ "llrint", TO_INTEGER, nullptr, nullptr, -1e12, 1e12 },
	{ "lround", TO_INTEGER, nullptr, nullptr, -1e6, 1e6 },
	{ "llround", TO_INTEGER, nullptr, nullptr, -1e12, 1e12 },
	{ "ilogb", ILOGB, nullptr, nullptr, -1e6, 1e6 },
	{ "logb", LOGB, nullptr, nullptr, -1e6, 1e6 },
	{ "frexp", FREXP, nullptr, nullptr, -1e6, 1e6 },
	{ "modf", MODF, nullptr, nullptr, -1e6, 1e6 },
	{ "nextafter", NEXTAFTER, nullptr, nullptr, -10.0, 10.0 },
	{ "nexttoward", NEXTAFTER, nullptr, nullptr, -10.0, 10.0 },
};

static void print_value(double x)
{
	if (isnan(x))
		printf("NAN");
	else if (isinf(x))
		printf("%sINFINITY", x < 0 ? "-" : "");
	else
		printf("%a", x);
}

static void emit(const struct function *f, const double *args)
{
	struct result out = {};

	if (f->kind == UNARY || f->kind == BINARY || f->kind == TERNARY || f->kind == SCALE || f->kind == REMQUO) {
		for (int d = 0; d < 4; ++d) {
			double offset = 0.0;
			long quotient = 0;

			out.value[d] = evaluate(f, args, directions[d], &out.flags[d], &offset, &quotient);
			if (d == 0) {
				out.delta = offset;
				out.aux = quotient;
			}
		}
		/* F.10.9.2, F.10.9.3: fmax(-0, +0) and fmin may return either zero */
		if ((strcmp(f->name, "fmax") == 0 || strcmp(f->name, "fmin") == 0) && args[0] == 0.0 && args[1] == 0.0)
			for (int d = 0; d < 4; ++d)
				out.flags[d] |= FLAG_ZERO_SIGN_ANY;
		/* C23 7.12.9: ceil, floor, trunc, round and nearbyint raise no inexact, which the harness ignores anyway */
	} else {
		exact_function(f, args, &out);
	}

	printf("\t{ ");
	for (int i = 0; i < 3; ++i) {
		print_value(args[i]);
		printf(", ");
	}
	printf("{ ");
	for (int d = 0; d < 4; ++d) {
		print_value(out.value[d]);
		printf(d < 3 ? ", " : " }, ");
	}
	print_value(out.delta);
	printf(", ");
	print_value(out.extra);
	printf(", %ld, { %d, %d, %d, %d } },\n", out.aux, out.flags[0], out.flags[1], out.flags[2], out.flags[3]);
}

int main(int argc, char **argv)
{
	if (argc != 2 || (strcmp(argv[1], "double") != 0 && strcmp(argv[1], "float") != 0)) {
		fprintf(stderr, "usage: libm-vectors double|float\n");
		return 1;
	}
	single = strcmp(argv[1], "float") == 0;
	precision = single ? 24 : 53;
	format_emin = single ? -148 : -1073;
	format_emax = single ? 128 : 1024;
	special_values = single ? special_values_float : special_values_double;
	/* The float list holds double literals: round each to float once */
	if (single)
		for (size_t i = 0; i < sizeof(special_values_float) / sizeof(special_values_float[0]); ++i)
			special_values_float[i] = in_format(special_values_float[i]);
	special_count = single ? sizeof(special_values_float) / sizeof(special_values_float[0]) : sizeof(special_values_double) / sizeof(special_values_double[0]);

	printf("/* Generated by tools/scripts/libm-vectors (MPFR %s) for %s; regenerate rather than edit (issue #95) */\n\n", mpfr_get_version(), argv[1]);
	for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		const struct function *f = &functions[i];
		double args[3] = { 0.0, 0.0, 0.0 };
		int randoms = 96;

		random_state = 0x2545f4914f6cdd1du ^ (uint64_t)i * 0x9e3779b97f4a7c15u;
		printf("static const struct libm_vector %s_vectors[] = {\n", f->name);
		switch (f->kind) {
			case BINARY:
			case REMQUO:
			case NEXTAFTER:
				for (size_t a = 0; a < sizeof(binary_specials) / sizeof(binary_specials[0]); ++a)
					for (size_t b = 0; b < sizeof(binary_specials) / sizeof(binary_specials[0]); ++b) {
						args[0] = special_values[binary_specials[a]];
						args[1] = special_values[binary_specials[b]];
						emit(f, args);
					}
				randoms = 128;
				for (int r = 0; r < randoms; ++r) {
					args[0] = random_argument(f, r);
					args[1] = (r % 4 == 0) ? in_format(args[0] * (1.0 + random_unit())) : random_argument(f, r + 1);
					emit(f, args);
				}
				break;
			case TERNARY:
				for (size_t a = 0; a < 5; ++a)
					for (size_t b = 0; b < 5; ++b)
						for (size_t c = 0; c < 4; ++c) {
							args[0] = special_values[binary_specials[a]];
							args[1] = special_values[binary_specials[b + 2]];
							args[2] = special_values[binary_specials[c * 2]];
							emit(f, args);
						}
				for (int r = 0; r < randoms; ++r) {
					args[0] = random_argument(f, r);
					args[1] = random_argument(f, r + 1);
					/* Cancellation: z close to -x * y */
					args[2] = (r % 3 == 0) ? in_format(-args[0] * args[1]) : random_argument(f, r + 2);
					emit(f, args);
				}
				break;
			case SCALE:
				for (size_t a = 0; a < special_count; ++a)
					for (int n = -1100; n <= 1100; n += 275) {
						args[0] = special_values[a];
						args[1] = n;
						emit(f, args);
					}
				for (int r = 0; r < randoms; ++r) {
					args[0] = random_argument(f, r);
					args[1] = (double)((int)(next_random() % 401) - 200);
					emit(f, args);
				}
				break;
			default:
				for (size_t a = 0; a < special_count; ++a) {
					args[0] = special_values[a];
					emit(f, args);
				}
				for (int r = 0; r < randoms; ++r) {
					args[0] = random_argument(f, r);
					emit(f, args);
				}
				/* Halfway and near-halfway arguments for the rounding functions */
				if (f->kind == TO_INTEGER || strcmp(f->name, "round") == 0 || strcmp(f->name, "rint") == 0 || strcmp(f->name, "nearbyint") == 0) {
					for (int k = -6; k <= 6; ++k) {
						args[0] = k + 0.5;
						emit(f, args);
					}
					args[0] = in_format(0x1.fffffffffffffp-2);
					emit(f, args);
				}
				break;
		}
		printf("};\n\n");
	}
	return 0;
}
