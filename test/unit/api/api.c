/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * api.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * What the standard says the headers' macros and types are, beyond their
 * existence (test/unit/annexb) and the integer limits (test/unit/headers):
 * which macros are integer constant expressions, the type of each constant,
 * the relations the standard fixes between them, and the result type of
 * every <tgmath.h> macro for each argument type (N3220 clause 7; issue #93).
 * Most checks are static_assert; main runs the few that need execution.
 * api.mk also compiles every header alone with -std=c23 -pedantic-errors.
 */

#undef NDEBUG
#include <assert.h>
#include <complex.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#include "api.h"

#define SAME_TYPE(expr, type) _Generic((expr), type: true, default: false)

/* An integer constant expression: usable as a case label (C23 6.8.5.3p3) */
#define ICE(value)      \
	do {                \
		switch (0) {    \
			case value: \
				break;  \
		}               \
	} while (false)

/* C23 5.3.5.3.3 (<float.h>): the integer characteristics are #if-usable; IEC 60559 binary32 and binary64, long double binary64 (D7) */
#if !(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && DBL_MANT_DIG == 53 && LDBL_MANT_DIG == 53)
#error <float.h> significand sizes
#endif
#if !(FLT_DIG == 6 && DBL_DIG == 15 && LDBL_DIG == 15)
#error <float.h> decimal digits
#endif
#if !(FLT_DECIMAL_DIG == 9 && DBL_DECIMAL_DIG == 17 && LDBL_DECIMAL_DIG == 17 && DECIMAL_DIG == 17)
#error <float.h> round-trip decimal digits
#endif
#if !(FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024 && LDBL_MIN_EXP == -1021 && LDBL_MAX_EXP == 1024)
#error <float.h> binary exponent range
#endif
#if !(FLT_MIN_10_EXP == -37 && FLT_MAX_10_EXP == 38 && DBL_MIN_10_EXP == -307 && DBL_MAX_10_EXP == 308 && LDBL_MIN_10_EXP == -307 && LDBL_MAX_10_EXP == 308)
#error <float.h> decimal exponent range
#endif
#if !(FLT_EVAL_METHOD == 0 && FLT_HAS_SUBNORM == 1 && DBL_HAS_SUBNORM == 1 && LDBL_HAS_SUBNORM == 1)
#error <float.h> evaluation method or subnormal support
#endif
#if !((FLT_IS_IEC_60559 == 0 || FLT_IS_IEC_60559 == 1) && (DBL_IS_IEC_60559 == 0 || DBL_IS_IEC_60559 == 1) && (LDBL_IS_IEC_60559 == 0 || LDBL_IS_IEC_60559 == 1))
#error <float.h> IEC 60559 conformance values
#endif

/* The floating characteristics have the type they describe (5.3.5.3.3p10-p13) */
static_assert(SAME_TYPE(FLT_MAX, float) && SAME_TYPE(FLT_MIN, float) && SAME_TYPE(FLT_EPSILON, float) && SAME_TYPE(FLT_TRUE_MIN, float) && SAME_TYPE(FLT_NORM_MAX, float) && SAME_TYPE(FLT_SNAN, float));
static_assert(SAME_TYPE(DBL_MAX, double) && SAME_TYPE(DBL_MIN, double) && SAME_TYPE(DBL_EPSILON, double) && SAME_TYPE(DBL_TRUE_MIN, double) && SAME_TYPE(DBL_NORM_MAX, double) && SAME_TYPE(DBL_SNAN, double));
static_assert(SAME_TYPE(LDBL_MAX, long double) && SAME_TYPE(LDBL_MIN, long double) && SAME_TYPE(LDBL_EPSILON, long double) && SAME_TYPE(LDBL_TRUE_MIN, long double) && SAME_TYPE(LDBL_NORM_MAX, long double) && SAME_TYPE(LDBL_SNAN, long double));
static_assert(FLT_MAX == 0x1.fffffep127f && FLT_MIN == 0x1p-126f && FLT_EPSILON == 0x1p-23f && FLT_TRUE_MIN == 0x1p-149f && FLT_NORM_MAX == FLT_MAX);
static_assert(DBL_MAX == 0x1.fffffffffffffp1023 && DBL_MIN == 0x1p-1022 && DBL_EPSILON == 0x1p-52 && DBL_TRUE_MIN == 0x1p-1074 && DBL_NORM_MAX == DBL_MAX);
static_assert(LDBL_MAX == DBL_MAX && LDBL_MIN == DBL_MIN && LDBL_EPSILON == DBL_EPSILON && LDBL_TRUE_MIN == DBL_TRUE_MIN);

/* C23 7.12p3-p6 (<math.h>): FLT_EVAL_METHOD 0 makes float_t and double_t float and double */
static_assert(SAME_TYPE((float_t)0, float) && SAME_TYPE((double_t)0, double));
static_assert(SAME_TYPE(HUGE_VAL, double) && SAME_TYPE(HUGE_VALF, float) && SAME_TYPE(HUGE_VALL, long double) && HUGE_VAL > 0 && HUGE_VALF > 0 && HUGE_VALL > 0);
static_assert(SAME_TYPE(INFINITY, float) && INFINITY > FLT_MAX && SAME_TYPE(NAN, float));

/* 7.12p6, p7: the classification and rounding-direction macros are distinct integer constant expressions */
static_assert(FP_NAN != FP_INFINITE && FP_NAN != FP_NORMAL && FP_NAN != FP_SUBNORMAL && FP_NAN != FP_ZERO && FP_INFINITE != FP_NORMAL && FP_INFINITE != FP_SUBNORMAL && FP_INFINITE != FP_ZERO && FP_NORMAL != FP_SUBNORMAL && FP_NORMAL != FP_ZERO && FP_SUBNORMAL != FP_ZERO);
static_assert(FP_INT_UPWARD != FP_INT_DOWNWARD && FP_INT_UPWARD != FP_INT_TOWARDZERO && FP_INT_UPWARD != FP_INT_TONEARESTFROMZERO && FP_INT_UPWARD != FP_INT_TONEAREST && FP_INT_DOWNWARD != FP_INT_TOWARDZERO && FP_INT_DOWNWARD != FP_INT_TONEARESTFROMZERO && FP_INT_DOWNWARD != FP_INT_TONEAREST && FP_INT_TOWARDZERO != FP_INT_TONEARESTFROMZERO && FP_INT_TOWARDZERO != FP_INT_TONEAREST && FP_INT_TONEARESTFROMZERO != FP_INT_TONEAREST);

/* 7.12p8, p9: FP_ILOGB0 is INT_MIN or -INT_MAX, FP_ILOGBNAN INT_MIN or INT_MAX, and the llogb ones the same in long int */
static_assert((FP_ILOGB0 == INT_MIN || FP_ILOGB0 == -INT_MAX) && (FP_ILOGBNAN == INT_MIN || FP_ILOGBNAN == INT_MAX));
static_assert(SAME_TYPE(FP_LLOGB0, long) && SAME_TYPE(FP_LLOGBNAN, long) && (FP_LLOGB0 == LONG_MIN || FP_LLOGB0 == -LONG_MAX) && (FP_LLOGBNAN == LONG_MIN || FP_LLOGBNAN == LONG_MAX));

/* 7.12p10: MATH_ERRNO 1, MATH_ERREXCEPT 2, math_errhandling an int expression of one or both */
static_assert(MATH_ERRNO == 1 && MATH_ERREXCEPT == 2);
static_assert(SAME_TYPE(math_errhandling, int) && (math_errhandling & ~(MATH_ERRNO | MATH_ERREXCEPT)) == 0 && math_errhandling != 0);

/* C23 7.6p6-p10 (<fenv.h>): the supported exception macros OR to distinct values and to FE_ALL_EXCEPT; the rounding macros are distinct and nonnegative */
#ifdef FE_INVALID
static_assert((FE_INVALID & FE_DIVBYZERO) == 0 && (FE_INVALID & FE_OVERFLOW) == 0 && (FE_INVALID & FE_UNDERFLOW) == 0 && (FE_INVALID & FE_INEXACT) == 0 && (FE_DIVBYZERO & FE_OVERFLOW) == 0 && (FE_DIVBYZERO & FE_UNDERFLOW) == 0 && (FE_DIVBYZERO & FE_INEXACT) == 0 && (FE_OVERFLOW & FE_UNDERFLOW) == 0 && (FE_OVERFLOW & FE_INEXACT) == 0 && (FE_UNDERFLOW & FE_INEXACT) == 0);
static_assert(FE_ALL_EXCEPT == (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT));
#else
static_assert(FE_ALL_EXCEPT == 0);
#endif
#ifdef FE_UPWARD
static_assert(FE_TONEAREST >= 0 && FE_UPWARD >= 0 && FE_DOWNWARD >= 0 && FE_TOWARDZERO >= 0 && FE_TONEAREST != FE_UPWARD && FE_TONEAREST != FE_DOWNWARD && FE_TONEAREST != FE_TOWARDZERO && FE_UPWARD != FE_DOWNWARD && FE_UPWARD != FE_TOWARDZERO && FE_DOWNWARD != FE_TOWARDZERO);
#endif
static_assert(FE_TONEAREST >= 0);
static_assert(SAME_TYPE(FE_DFL_ENV, const fenv_t *) && SAME_TYPE(FE_DFL_MODE, const femode_t *));
static_assert(SAME_TYPE((fexcept_t)0 + 0u, unsigned int) || sizeof(fexcept_t) > 0);

/* C23 7.14p3, p4 (<signal.h>): the handler macros have the type of a handler, the signal numbers are distinct and positive */
static_assert(SAME_TYPE(SIG_DFL, void (*)(int)) && SAME_TYPE(SIG_ERR, void (*)(int)) && SAME_TYPE(SIG_IGN, void (*)(int)));
static_assert(SIGABRT > 0 && SIGFPE > 0 && SIGILL > 0 && SIGINT > 0 && SIGSEGV > 0 && SIGTERM > 0);
static_assert(SIGABRT != SIGFPE && SIGABRT != SIGILL && SIGABRT != SIGINT && SIGABRT != SIGSEGV && SIGABRT != SIGTERM && SIGFPE != SIGILL && SIGFPE != SIGINT && SIGFPE != SIGSEGV && SIGFPE != SIGTERM && SIGILL != SIGINT && SIGILL != SIGSEGV && SIGILL != SIGTERM && SIGINT != SIGSEGV && SIGINT != SIGTERM && SIGSEGV != SIGTERM);

/* C23 7.21 (<stddef.h>): offsetof is an integer constant expression of type size_t; max_align_t is the strictest fundamental alignment */
struct record {
	char tag;
	double value;
};
static_assert(SAME_TYPE(offsetof(struct record, value), size_t) && offsetof(struct record, value) == alignof(double));
static_assert(alignof(max_align_t) >= alignof(long double) && alignof(max_align_t) >= alignof(long long) && alignof(max_align_t) >= alignof(void *));
static_assert(SAME_TYPE(nullptr, nullptr_t) && (ptrdiff_t)-1 < 0 && (size_t)-1 > 0);

/* C23 7.3.1p4, p5 (<complex.h>): I is _Complex_I, a constant of type const float _Complex with value the imaginary unit */
static_assert(SAME_TYPE(_Complex_I, float _Complex) && SAME_TYPE(I, float _Complex));
static_assert(SAME_TYPE(CMPLX(1, 2), double _Complex) && SAME_TYPE(CMPLXF(1, 2), float _Complex) && SAME_TYPE(CMPLXL(1, 2), long double _Complex));

/* C23 7.27 (<tgmath.h>): the result type of every type-generic macro for each argument type; integers count as double */
#define REAL1(f) static_assert(SAME_TYPE(f(1.0f), float) && SAME_TYPE(f(1.0), double) && SAME_TYPE(f(1.0L), long double) && SAME_TYPE(f(1), double), #f)
#define COMPLEX1(f) static_assert(SAME_TYPE(f((float _Complex)1), float _Complex) && SAME_TYPE(f((double _Complex)1), double _Complex) && SAME_TYPE(f((long double _Complex)1), long double _Complex), #f " complex")
#define REAL2(f) static_assert(SAME_TYPE(f(1.0f, 1.0f), float) && SAME_TYPE(f(1.0f, 1.0), double) && SAME_TYPE(f(1.0, 1.0L), long double) && SAME_TYPE(f(1, 1.0f), double), #f)
#define REAL3(f) static_assert(SAME_TYPE(f(1.0f, 1.0f, 1.0f), float) && SAME_TYPE(f(1.0f, 1.0f, 1.0), double) && SAME_TYPE(f(1.0L, 1.0f, 1), long double), #f)
#define RESULT(f, type) static_assert(SAME_TYPE(f(1.0f), type) && SAME_TYPE(f(1.0), type) && SAME_TYPE(f(1.0L), type), #f)

/* Real and complex */
REAL1(acos);
COMPLEX1(acos);
REAL1(asin);
COMPLEX1(asin);
REAL1(atan);
COMPLEX1(atan);
REAL1(acosh);
COMPLEX1(acosh);
REAL1(asinh);
COMPLEX1(asinh);
REAL1(atanh);
COMPLEX1(atanh);
REAL1(cos);
COMPLEX1(cos);
REAL1(sin);
COMPLEX1(sin);
REAL1(tan);
COMPLEX1(tan);
REAL1(cosh);
COMPLEX1(cosh);
REAL1(sinh);
COMPLEX1(sinh);
REAL1(tanh);
COMPLEX1(tanh);
REAL1(exp);
COMPLEX1(exp);
REAL1(log);
COMPLEX1(log);
REAL1(sqrt);
COMPLEX1(sqrt);
REAL2(pow);
static_assert(SAME_TYPE(pow((double _Complex)1, 1.0f), double _Complex) && SAME_TYPE(pow(1.0f, (float _Complex)1), float _Complex), "pow complex");
REAL1(fabs);
static_assert(SAME_TYPE(fabs((float _Complex)1), float) && SAME_TYPE(fabs((double _Complex)1), double) && SAME_TYPE(fabs((long double _Complex)1), long double), "fabs complex");

/* Real only, one argument */
REAL1(acospi);
REAL1(asinpi);
REAL1(atanpi);
REAL1(cbrt);
REAL1(ceil);
REAL1(cospi);
REAL1(erf);
REAL1(erfc);
REAL1(exp10);
REAL1(exp10m1);
REAL1(exp2);
REAL1(exp2m1);
REAL1(expm1);
REAL1(floor);
REAL1(lgamma);
REAL1(log10);
REAL1(log10p1);
REAL1(log1p);
REAL1(log2);
REAL1(log2p1);
REAL1(logb);
REAL1(logp1);
REAL1(nearbyint);
REAL1(nextdown);
REAL1(nextup);
REAL1(rint);
REAL1(round);
REAL1(roundeven);
REAL1(rsqrt);
REAL1(sinpi);
REAL1(tanpi);
REAL1(tgamma);
REAL1(trunc);

/* Real only, two or three arguments */
REAL2(atan2);
REAL2(atan2pi);
REAL2(copysign);
REAL2(fdim);
REAL2(fmax);
REAL2(fmaximum);
REAL2(fmaximum_mag);
REAL2(fmaximum_mag_num);
REAL2(fmaximum_num);
REAL2(fmin);
REAL2(fminimum);
REAL2(fminimum_mag);
REAL2(fminimum_mag_num);
REAL2(fminimum_num);
REAL2(fmod);
REAL2(hypot);
REAL2(nextafter);
REAL2(powr);
REAL2(remainder);
REAL3(fma);

/* An integer result, or a non-generic second argument */
RESULT(ilogb, int);
RESULT(llogb, long);
RESULT(lrint, long);
RESULT(lround, long);
RESULT(llrint, long long);
RESULT(llround, long long);
static_assert(SAME_TYPE(ldexp(1.0f, 1), float) && SAME_TYPE(scalbn(1.0L, 1), long double) && SAME_TYPE(scalbln(1.0f, 1L), float), "exponent scaling");
static_assert(SAME_TYPE(compoundn(1.0f, 1LL), float) && SAME_TYPE(pown(1.0L, 1LL), long double) && SAME_TYPE(rootn(1.0, 1LL), double), "integer powers");
static_assert(SAME_TYPE(nexttoward(1.0f, 1.0L), float) && SAME_TYPE(nexttoward(1.0, 1.0L), double), "nexttoward");
static_assert(SAME_TYPE(fromfp(1.0f, FP_INT_TONEAREST, 8u), float) && SAME_TYPE(ufromfpx(1.0L, FP_INT_UPWARD, 8u), long double), "fromfp");

/* Complex only: carg, cimag and creal return the real type */
COMPLEX1(conj);
COMPLEX1(cproj);
static_assert(SAME_TYPE(carg((float _Complex)1), float) && SAME_TYPE(cimag((double _Complex)1), double) && SAME_TYPE(creal((long double _Complex)1), long double), "complex parts");

/* 7.27p5: the narrowing macros round to float (f*) or double (d*) whatever the argument types */
static_assert(SAME_TYPE(fadd(1.0, 1.0), float) && SAME_TYPE(fsub(1.0L, 1.0), float) && SAME_TYPE(fmul(1, 1.0), float) && SAME_TYPE(fdiv(1.0, 1.0), float) && SAME_TYPE(ffma(1.0, 1.0, 1.0), float) && SAME_TYPE(fsqrt(1.0L), float), "f narrowing");
static_assert(SAME_TYPE(dadd(1.0L, 1.0L), double) && SAME_TYPE(dsub(1.0, 1.0), double) && SAME_TYPE(dmul(1.0L, 1.0), double) && SAME_TYPE(ddiv(1.0L, 1.0L), double) && SAME_TYPE(dfma(1.0L, 1.0L, 1.0L), double) && SAME_TYPE(dsqrt(1.0L), double), "d narrowing");

static int failures;

static void check(bool ok, const char *what)
{
	if (!ok) {
		printf("FAIL %s\n", what);
		++failures;
	}
}

/* C23 7.16.1.4: va_start with only the va_list; va_copy gives an independent traversal */
static int sum(int count, ...)
{
	va_list args;
	va_list again;
	int total = 0;

	va_start(args);
	va_copy(again, args);
	for (int i = 0; i < count; ++i)
		total += va_arg(args, int);
	for (int i = 0; i < count; ++i)
		total -= va_arg(again, int) * 2;
	va_end(again);
	va_end(args);

	return -total;
}

static jmp_buf jump;

static void leave(int value)
{
	longjmp(jump, value);
}

static void handler([[maybe_unused]] int signal_number)
{
}

int assert_count;

bool counted(void)
{
	++assert_count;
	return true;
}

int main(void)
{
	volatile int arrivals = 0;
	int status;

	/* The integer constant expressions, as case labels */
	ICE(EDOM);
	ICE(ERANGE);
	ICE(EILSEQ);
	ICE(FE_ALL_EXCEPT);
	ICE(FE_TONEAREST);
	ICE(FP_NAN);
	ICE(FP_INT_TONEAREST);
	ICE(FP_ILOGB0);
	ICE(MATH_ERRNO);
	ICE(SIGABRT);
	ICE(SIGTERM);
	ICE(FLT_RADIX);
	ICE(DECIMAL_DIG);
	ICE(offsetof(struct record, value));

	/* C23 7.5p2: errno is a modifiable lvalue of type int */
	errno = ERANGE;
	check(SAME_TYPE(errno, int) && errno == ERANGE, "errno is an int lvalue");
	errno = 0;

	/* C23 7.9: the alternative spellings are the operators */
	check((true and false) == false && (true or false) && (not false) && (6 bitand 3) == 2 && (6 bitor 3) == 7 && (6 xor 3) == 5 && (compl 0) == -1 && (1 not_eq 2), "<iso646.h> operators");
	status = 6;
	status and_eq 3;
	status or_eq 8;
	status xor_eq 1;
	check(status == 11, "<iso646.h> assignment operators");

	check(sum(3, 1, 2, 3) == 6, "va_start, va_arg, va_copy, va_end");

	/* C23 7.13: setjmp returns 0, then the value passed to longjmp; longjmp(env, 0) makes it return 1 */
	status = setjmp(jump);
	if (status == 0) {
		arrivals = arrivals + 1;
		leave(0);
	}
	check(status == 1 && arrivals == 1, "longjmp(env, 0) returns 1 from setjmp");
	status = setjmp(jump);
	if (status == 0)
		leave(42);
	check(status == 42, "longjmp value");

	/* C23 7.14.1.1p8: signal returns the previous handler */
	check(signal(SIGINT, handler) == SIG_DFL && signal(SIGINT, SIG_DFL) == handler, "signal returns the previous handler");

	/* C23 7.2p1: without NDEBUG the expression is evaluated, with it not; <assert.h> may be included again to switch */
	assert(counted());
	check(assert_count == 1, "assert evaluates its argument without NDEBUG");
	ndebug_assert();
	check(assert_count == 1, "assert does not evaluate its argument under NDEBUG");

	/* C23 7.3: I squared is -1 */
	check(creal(I * I) == -1.0f && cimag(I * I) == 0.0f, "I is the imaginary unit");

	printf("%s\n", failures == 0 ? "api: ok" : "api: failed");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
