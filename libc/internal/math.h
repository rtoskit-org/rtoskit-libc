/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * math.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_MATH_H
#define _INTERNAL_MATH_H

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

/* The FPU of the build (libc/internal/fpu/${LIBC_FPU}, on the include path through CPPFLAGS) */
#include <libc-fpu.h>

/*
 * Helpers of libm (docs/research/fenv-libm.md). Values are taken apart
 * through unions (well defined in C). Exceptions are raised by arithmetic,
 * so on a core without them (D44) nothing happens.
 */

/* The l functions call the double ones (PLAN D7) */
static_assert(LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP && LDBL_MIN_EXP == DBL_MIN_EXP, "long double must have the format of double");

#define __LIBC_DOUBLE_SIGN 0x8000000000000000ULL
#define __LIBC_DOUBLE_EXPONENT 0x7ff0000000000000ULL
#define __LIBC_DOUBLE_FRACTION 0x000fffffffffffffULL
#define __LIBC_DOUBLE_QUIET 0x0008000000000000ULL
#define __LIBC_DOUBLE_BIAS 1023

#define __LIBC_FLOAT_SIGN 0x80000000U
#define __LIBC_FLOAT_EXPONENT 0x7f800000U
#define __LIBC_FLOAT_FRACTION 0x007fffffU
#define __LIBC_FLOAT_QUIET 0x00400000U
#define __LIBC_FLOAT_BIAS 127

union __libc_double {
	double value;
	uint64_t bits;
};

union __libc_float {
	float value;
	uint32_t bits;
};

static inline uint64_t __libc_double_bits(double x)
{
	return (union __libc_double){ .value = x }.bits;
}

static inline double __libc_double_from(uint64_t bits)
{
	return (union __libc_double){ .bits = bits }.value;
}
/*
 * 64-bit integers to double, correctly rounded as the conversion is. The
 * generic files convert natively; the fpv5-dp layer, whose FPU converts
 * 32-bit integers only, adds two exact 32-bit conversions instead of linking
 * the soft-float library (libgcc keeps __aeabi_ul2d and __aeabi_l2d in the
 * object of the soft double addition). PLAN D55.
 */
double __libc_u64_to_double(uint64_t x);
double __libc_i64_to_double(int64_t x);

/*
 * An integral double in [-2^63, 2^63) to int64_t, without the compiler's
 * 64-bit conversion helpers (__aeabi_d2lz and __aeabi_d2ulz): from the bits
 * in the generic file, through two 32-bit conversions in the fpv5-dp layer
 * (PLAN D55).
 */
int64_t __libc_double_to_i64(double x);

static inline uint32_t __libc_float_bits(float x)
{
	return (union __libc_float){ .value = x }.bits;
}

static inline float __libc_float_from(uint32_t bits)
{
	return (union __libc_float){ .bits = bits }.value;
}

/* The biased exponent field */
static inline int __libc_double_exponent(uint64_t bits)
{
	return (int)((bits >> 52) & 0x7ff);
}

static inline int __libc_float_exponent(uint32_t bits)
{
	return (int)((bits >> 23) & 0xff);
}

/*
 * An argument whose function value lies so near the point halfway between
 * two floats that the double result cannot decide its rounding (PLAN D98,
 * docs/research/float-hard-cases.md), and its correctly rounded result.
 */
struct __libc_float_case {
	uint32_t x;
	uint32_t result;
};

/*
 * The double result y of f(x) rounded to float, f's hard cases included:
 * x looked up in cases, out of line in libc/math/__libc_float_case.c, with
 * y (float)y when x is not one of them.
 */
float __libc_float_case(double y, float x, const struct __libc_float_case *cases, size_t count);

/*
 * The double result y of f(x) rounded to float, also for f's hard cases.
 * Only a y within 2^8 double ulps of a float halfway point (its 29 bits below
 * float precision within 256 of 2^28) looks x up; every hard case found has
 * its y exactly on one. The cases have normal results, where those 29 bits
 * are the ones below float precision.
 */
static inline float __libc_float_round(double y, float x, const struct __libc_float_case *cases, size_t count)
{
	if (((uint32_t)__libc_double_bits(y) & 0x1fffffff) - (0x10000000 - 0x100) < 0x200)
		return __libc_float_case(y, x, cases, count);

	return (float)y;
}

/*
 * Raise one exception as the operation would. Out of line in libc/fenv,
 * where the <fenv.h> variant decides: an arithmetic operation where the FPU
 * records exceptions, nothing where none is recorded, so a program on such a
 * core links no floating-point arithmetic just to raise one (PLAN D55).
 */
void __libc_math_raise_invalid(void);
void __libc_math_raise_divbyzero(void);

/* Overflow and underflow come with inexact, as any overflowing or underflowing operation */
void __libc_math_raise_overflow(void);
void __libc_math_raise_underflow(void);
void __libc_math_raise_inexact(void);

/*
 * The results of the special cases, raising their exception: out of line in
 * libc/fenv, where the <fenv.h> variant decides. Where exceptions and
 * rounding directions exist they come from the operation itself (0/0,
 * sign/0, sign * tiny * tiny); elsewhere they are constants (PLAN D55).
 */

/* A quiet NaN, raising invalid: the result of a domain error (math_errhandling is MATH_ERREXCEPT) */
double __libc_math_invalid(void);
float __libc_math_invalidf(void);

/* The core of fromfp, ufromfp, fromfpx and ufromfpx */
double __libc_fromfp(double x, int rnd, unsigned int width, bool is_unsigned, bool exact);

/* The remainder core of fmod, remainder and remquo */
double __libc_remainder(double x, double y, bool nearest, int *quotient);

/*
 * The single-precision layer (libc/math/sp, PLAN D56): the float functions
 * computed in float only, for cores whose FPU has no double precision.
 */
float __libc_remainderf(float x, float y, bool nearest, int *quotient);
float __libc_fromfpf(float x, int rnd, unsigned int width, bool is_unsigned, bool exact);

/* An integral float in [-2^63, 2^63) to int64_t, from its bits (no __aeabi_f2lz, which computes in double) */
int64_t __libc_float_to_i64(float x);

/*
 * An exact value for one final rounding (docs/research/fenv-libm.md):
 * (-1)^negative * (high * 2^64 + low + e) * 2^exponent, where e is 0 when
 * sticky is false and strictly between 0 and 1 otherwise.
 */
struct __libc_exact {
	bool negative;
	bool sticky;
	int exponent;
	uint64_t high;
	uint64_t low;
};

/* The value rounded once in the current direction, with the right exceptions; it must not be zero */
double __libc_exact_to_double(const struct __libc_exact *value);
float __libc_exact_to_float(const struct __libc_exact *value);

/*
 * (-1)^product_negative * mx * my * 2^product_exponent +
 * (-1)^addend_negative * mz * 2^addend_exponent, exactly (mx, my and mz
 * below 2^62, mx and my not zero); false when the sum is exactly zero.
 */
bool __libc_exact_fma(struct __libc_exact *result, bool product_negative, uint64_t mx, uint64_t my, int product_exponent, bool addend_negative, uint64_t mz, int addend_exponent);

/* x = mantissa * 2^*exponent with the mantissa in [2^52, 2^53); x finite and not zero */
static inline uint64_t __libc_double_split(double x, int *exponent)
{
	uint64_t bits = __libc_double_bits(x);
	int biased = __libc_double_exponent(bits);
	uint64_t mantissa = bits & __LIBC_DOUBLE_FRACTION;

	if (biased != 0) {
		mantissa |= 1ULL << 52;
	} else {
		int shift = __builtin_clzll(mantissa) - 11;

		mantissa <<= shift;
		biased = 1 - shift;
	}

	*exponent = biased - __LIBC_DOUBLE_BIAS - 52;
	return mantissa;
}

/* x = mantissa * 2^*exponent with the mantissa in [2^23, 2^24); x finite and not zero */
static inline uint64_t __libc_float_split(float x, int *exponent)
{
	uint32_t bits = __libc_float_bits(x);
	int biased = __libc_float_exponent(bits);
	uint32_t mantissa = bits & __LIBC_FLOAT_FRACTION;

	if (biased != 0) {
		mantissa |= 1U << 23;
	} else {
		int shift = __builtin_clz(mantissa) - 8;

		mantissa <<= shift;
		biased = 1 - shift;
	}

	*exponent = biased - __LIBC_FLOAT_BIAS - 23;
	return mantissa;
}

/*
 * floor(sqrt(high * 2^64 + low)) for a radicand of at most 2 * digits bits
 * (digits at most 60), digit by digit; *inexact tells whether it was not a
 * perfect square.
 */
static inline uint64_t __libc_isqrt(uint64_t high, uint64_t low, int digits, bool *inexact)
{
	uint64_t root = 0;
	uint64_t remainder = 0;

	for (int i = digits - 1; i >= 0; --i) {
		int shift = 2 * i;
		uint64_t pair = shift >= 64 ? (high >> (shift - 64)) & 3 : (low >> shift) & 3;
		uint64_t trial;

		remainder = (remainder << 2) | pair;
		trial = (root << 2) | 1;
		root <<= 1;
		if (remainder >= trial) {
			remainder -= trial;
			root |= 1;
		}
	}

	*inexact = remainder != 0;
	return root;
}

/*
 * Double-double arithmetic for the exponential and logarithm cores
 * (docs/research/libm-exp-log.md): a value hi + lo with |lo| at most half an
 * ulp of hi. The error-free transformations are exact only when rounding to
 * nearest, so the cores select that direction (__libc_math_nearest, D47).
 * Products split their factors (Veltkamp), so they need no fma; the factors
 * must stay below 2^995.
 */
struct __libc_dd {
	double hi;
	double lo;
};

/* a + b exactly, for any a and b */
static inline struct __libc_dd __libc_dd_sum(double a, double b)
{
	double sum = a + b;
	double bb = sum - a;

	return (struct __libc_dd){ .hi = sum, .lo = (a - (sum - bb)) + (b - bb) };
}

/* a + b exactly, when |a| >= |b| (or a is zero) */
static inline struct __libc_dd __libc_dd_fast_sum(double a, double b)
{
	double sum = a + b;

	return (struct __libc_dd){ .hi = sum, .lo = b - (sum - a) };
}

static inline void __libc_dd_split(double a, double *high, double *low)
{
	double c = 0x1.0000002p27 * a;

	*high = c - (c - a);
	*low = a - *high;
}

/* a * b exactly: one fma where the FPU has it (libc-fpu.h, PLAN D83), else Dekker's split products */
static inline struct __libc_dd __libc_dd_product(double a, double b)
{
	double product = a * b;
	double ah;
	double al;
	double bh;
	double bl;

	if (__libc_fpu_fma)
		return (struct __libc_dd){ .hi = product, .lo = __builtin_fma(a, b, -product) };

	__libc_dd_split(a, &ah, &al);
	__libc_dd_split(b, &bh, &bl);

	return (struct __libc_dd){ .hi = product, .lo = ((ah * bh - product) + ah * bl + al * bh) + al * bl };
}

/* (a.hi + a.lo) * (b.hi + b.lo), to about 2^-100 */
static inline struct __libc_dd __libc_dd_multiply(struct __libc_dd a, struct __libc_dd b)
{
	struct __libc_dd product = __libc_dd_product(a.hi, b.hi);

	return __libc_dd_fast_sum(product.hi, product.lo + (a.hi * b.lo + a.lo * b.hi));
}

static inline struct __libc_dd __libc_dd_negate(struct __libc_dd a)
{
	return (struct __libc_dd){ .hi = -a.hi, .lo = -a.lo };
}

/* ln 2, ln 10, 1/ln 2, 1/ln 10 as double-double */
#define __LIBC_LN2 ((struct __libc_dd){ .hi = 0x1.62e42fefa39efp-1, .lo = 0x1.abc9e3b39803fp-56 })
#define __LIBC_LN10 ((struct __libc_dd){ .hi = 0x1.26bb1bbb55516p+1, .lo = -0x1.f48ad494ea3e9p-53 })
#define __LIBC_INV_LN2 ((struct __libc_dd){ .hi = 0x1.71547652b82fep+0, .lo = 0x1.777d0ffda0d24p-56 })
#define __LIBC_INV_LN10 ((struct __libc_dd){ .hi = 0x1.bcb7b1526e50ep-2, .lo = 0x1.95355baaafad3p-57 })

/*
 * exp(xh + xl) = 2^*exponent * (result.hi + result.lo), to about 2^-64,
 * for |xh| below 746.
 */
struct __libc_dd __libc_exp_core(double xh, double xl, int *exponent);

/* 2^exponent * (value.hi + value.lo), rounded once in the current direction (twice for subnormal results) */
double __libc_exp_scale(struct __libc_dd value, int exponent);

/* expm1(zh + zl) for finite zh in (-45, 710) */
double __libc_expm1_core(double zh, double zl);

/* log(x) for positive finite x, to about 2^-64 (relative near 1) */
struct __libc_dd __libc_log_core(double x);

/* log(1 + x) for finite x > -1 with |x| at least 2^-54 */
struct __libc_dd __libc_log1p_core(double x);

/* exp(n * (log.hi + log.lo)) for integer n, signed: the core of pown and compoundn */
double __libc_pow_integer(struct __libc_dd log, long long n, bool negative);

/* +-infinity with overflow; inline, as the multiplication links no more than the caller has (PLAN D55) */
static inline double __libc_math_overflow(double sign)
{
	volatile double huge = 0x1p1023;

	return sign * huge * huge;
}

/* Zero with the sign of sign, rounded in the current direction, raising underflow (out of line, see above) */
double __libc_math_underflow(double sign);

/* Infinity with the sign of sign, raising divide-by-zero (out of line, see above) */
double __libc_math_divbyzero(double sign);

/*
 * The double-double algorithms (TwoSum, the exact product, the cores) are
 * exact only when rounding to nearest: the cores switch to it and restore
 * the caller's direction, which then decides the last rounding.
 */
static inline int __libc_math_nearest(void)
{
	int mode = fegetround();

	if (mode != FE_TONEAREST)
		fesetround(FE_TONEAREST);
	return mode;
}

static inline void __libc_math_restore(int mode)
{
	if (mode != FE_TONEAREST)
		fesetround(mode);
}

/*
 * f(x) for a tiny x where f(x) = x + e, 0 < |e| < ulp(x) / 2 and e has the
 * sign of direction: x scaled by 2^100, nudged by 2^-1074 toward f(x) and
 * scaled back, so the current rounding direction picks x or its neighbour
 * and raises inexact; underflow for a subnormal x (tininess before rounding).
 */
static inline double __libc_math_tiny(double x, double direction)
{
	if (x == 0)
		return x;
	if (fabs(x) < 0x1p-1022)
		__libc_math_raise_underflow();

	return (x * 0x1p100 + copysign(0x1p-1074, direction)) * 0x1p-100;
}

/*
 * The float version, for |x| below 2^-30: the nudge of 2^-40 |x| is exact in
 * double and well inside half a float ulp, so the conversion rounds in the
 * current direction and raises inexact, and underflow for a subnormal result.
 */
static inline float __libc_math_tinyf(float x, double direction)
{
	if (x == 0)
		return x;
	/*
	 * At +-FLT_MIN a nudge toward zero is tiny before rounding (Arm's rule)
	 * even when it rounds back to FLT_MIN: return x with inexact only, unless
	 * the direction (FLT_ROUNDS: 0 toward zero, 1 nearest, 2 up, 3 down)
	 * takes the result below FLT_MIN. signbit is only zero or nonzero (C23
	 * 7.12.3.6), and its nonzero values differ between float and double.
	 */
	if (fabsf(x) == FLT_MIN && !signbit(x) != !signbit(direction) && (FLT_ROUNDS == 1 || FLT_ROUNDS == (x > 0 ? 2 : 3))) {
		__libc_math_raise_inexact();
		return x;
	}

	return (float)((double)x + copysign(fabs((double)x) * 0x1p-40, direction));
}

/* (a.hi + a.lo) + (b.hi + b.lo), to about 2^-105 */
static inline struct __libc_dd __libc_dd_add(struct __libc_dd a, struct __libc_dd b)
{
	struct __libc_dd sum = __libc_dd_sum(a.hi, b.hi);

	return __libc_dd_fast_sum(sum.hi, sum.lo + a.lo + b.lo);
}

/* (a.hi + a.lo) / (b.hi + b.lo), to about 2^-100: the quotient corrected by the exact remainder */
static inline struct __libc_dd __libc_dd_divide(struct __libc_dd a, struct __libc_dd b)
{
	double quotient = a.hi / b.hi;
	struct __libc_dd product = __libc_dd_product(quotient, b.hi);

	return __libc_dd_fast_sum(quotient, ((((a.hi - product.hi) - product.lo) + a.lo) - quotient * b.lo) / b.hi);
}

/* 2/sqrt(pi) and 1/sqrt(pi) as float-float */
#define __LIBC_TWO_SQRTPIF ((struct __libc_ff){ .hi = 0x1.20dd76p0f, .lo = -0x1.f7ac92p-25f })
#define __LIBC_INV_SQRTPIF ((struct __libc_ff){ .hi = 0x1.20dd76p-1f, .lo = -0x1.f7ac92p-26f })

/* ln pi, 1/12 and ln(2 pi)/2 as float-float */
#define __LIBC_LN_PIF ((struct __libc_ff){ .hi = 0x1.250d04p0f, .lo = 0x1.1cf438p-25f })
#define __LIBC_ONE_TWELFTHF ((struct __libc_ff){ .hi = 0x1.555556p-4f, .lo = -0x1.555556p-29f })
#define __LIBC_HALF_LN_2PIF ((struct __libc_ff){ .hi = 0x1.d67f1cp-1f, .lo = 0x1.0c97d6p-26f })

/* sqrt(a.hi + a.lo) for a non-negative a, corrected by the residual (a - s^2)/(2s) */
static inline struct __libc_dd __libc_dd_sqrt(struct __libc_dd a)
{
	double root = sqrt(a.hi);
	struct __libc_dd square = __libc_dd_product(root, root);

	if (root == 0)
		return (struct __libc_dd){ .hi = 0.0, .lo = 0.0 };

	return __libc_dd_fast_sum(root, (((a.hi - square.hi) - square.lo) + a.lo) / (2.0 * root));
}

/* pi, pi/2 and 1/pi as double-double */
#define __LIBC_PI ((struct __libc_dd){ .hi = 0x1.921fb54442d18p+1, .lo = 0x1.1a62633145c07p-53 })
#define __LIBC_PI_2 ((struct __libc_dd){ .hi = 0x1.921fb54442d18p+0, .lo = 0x1.1a62633145c07p-54 })
#define __LIBC_INV_PI ((struct __libc_dd){ .hi = 0x1.45f306dc9c883p-2, .lo = -0x1.6b01ec5417056p-56 })

/*
 * Trigonometric and hyperbolic cores (docs/research/libm-trig.md). They
 * expect rounding to nearest (__libc_math_nearest), as the callers set it.
 */

/* x = n pi/2 + r with |r| about pi/4 at most; returns n mod 4 and r as a double-double, for finite x */
int __libc_rem_pio2(double x, struct __libc_dd *r);

/* sin and cos of r.hi + r.lo for |r| at most about pi/4, to about 2^-60 */
struct __libc_dd __libc_sin_core(struct __libc_dd r);
struct __libc_dd __libc_cos_core(struct __libc_dd r);

/* The angle of (x, y) in [-pi, pi] for finite, not both zero, double-double coordinates below 2^990 */
struct __libc_dd __libc_atan2_core(struct __libc_dd y, struct __libc_dd x);

/* log(1 + u) for a double-double u > -1 */
struct __libc_dd __libc_log1p_dd(struct __libc_dd u);

/* x * c rounded once in the current direction, also when the product is subnormal */
static inline double __libc_dd_scale_product(double x, struct __libc_dd c)
{
	struct __libc_dd product;
	int mode = __libc_math_nearest();

	product = __libc_dd_product(x * 0x1p200, c.hi);
	product = __libc_dd_fast_sum(product.hi, product.lo + x * 0x1p200 * c.lo);
	__libc_math_restore(mode);

	return __libc_exp_scale(product, -200);
}

/* sinh and cosh of |x| < 1, and exp(x) for |x| <= 40, as double-doubles (rounding to nearest expected) */
struct __libc_dd __libc_sinh_core(double x);
struct __libc_dd __libc_cosh_core(double x);
struct __libc_dd __libc_exp_dd(double x);

/* Special functions (docs/research/libm-special.md); rounding to nearest is expected */

/* erf(x) for |x| < 1/2, erfc(x) for 15/32 <= x < 129/32 (from a table), and erfc(x) = 2^*exponent * result for 4 <= x < 27.3 */
struct __libc_dd __libc_erf_series(double x);
struct __libc_dd __libc_erfc_table(double x);
struct __libc_dd __libc_erfc_tail(double x, int *exponent);

/* log|gamma(x)| for finite x that is not a pole (zero or a negative integer) and below 2^60; *sign is the sign of gamma(x) */
struct __libc_dd __libc_lgamma_core(double x, int *sign);

/*
 * Float-float arithmetic (PLAN D57): the double-double algorithms above at
 * single precision, for the float functions of the sp-nearest layer on cores
 * whose FPU has no double precision. Exact only when rounding to nearest,
 * which is the only direction those cores have (D44).
 */
struct __libc_ff {
	float hi;
	float lo;
};

/* a + b exactly, for any a and b */
static inline struct __libc_ff __libc_ff_sum(float a, float b)
{
	float sum = a + b;
	float bb = sum - a;

	return (struct __libc_ff){ .hi = sum, .lo = (a - (sum - bb)) + (b - bb) };
}

/* a + b exactly, when |a| >= |b| (or a is zero) */
static inline struct __libc_ff __libc_ff_fast_sum(float a, float b)
{
	float sum = a + b;

	return (struct __libc_ff){ .hi = sum, .lo = b - (sum - a) };
}

/* Veltkamp's split into two halves of at most 12 bits: 4097 is 2^12 + 1 */
static inline void __libc_ff_split(float a, float *high, float *low)
{
	float c = 0x1.002p12f * a;

	*high = c - (c - a);
	*low = a - *high;
}

/*
 * a * b exactly, for |a| and |b| below 2^115 and a product clear of the
 * subnormal range: the error of the rounded product is one fma where the
 * FPU has it (libc-fpu.h, PLAN D82), else Dekker's split products
 */
static inline struct __libc_ff __libc_ff_product(float a, float b)
{
	float product = a * b;
	float ah;
	float al;
	float bh;
	float bl;

	if (__libc_fpu_fmaf)
		return (struct __libc_ff){ .hi = product, .lo = __builtin_fmaf(a, b, -product) };

	__libc_ff_split(a, &ah, &al);
	__libc_ff_split(b, &bh, &bl);

	return (struct __libc_ff){ .hi = product, .lo = ((ah * bh - product) + ah * bl + al * bh) + al * bl };
}

/* (a.hi + a.lo) * (b.hi + b.lo), to about 2^-44 */
static inline struct __libc_ff __libc_ff_multiply(struct __libc_ff a, struct __libc_ff b)
{
	struct __libc_ff product = __libc_ff_product(a.hi, b.hi);

	return __libc_ff_fast_sum(product.hi, product.lo + (a.hi * b.lo + a.lo * b.hi));
}

/* (a.hi + a.lo) + (b.hi + b.lo), to about 2^-45 */
static inline struct __libc_ff __libc_ff_add(struct __libc_ff a, struct __libc_ff b)
{
	struct __libc_ff sum = __libc_ff_sum(a.hi, b.hi);

	return __libc_ff_fast_sum(sum.hi, sum.lo + a.lo + b.lo);
}

/* (a.hi + a.lo) / (b.hi + b.lo), to about 2^-44: the quotient corrected by the exact remainder */
static inline struct __libc_ff __libc_ff_divide(struct __libc_ff a, struct __libc_ff b)
{
	float quotient = a.hi / b.hi;
	struct __libc_ff product = __libc_ff_product(quotient, b.hi);

	return __libc_ff_fast_sum(quotient, ((((a.hi - product.hi) - product.lo) + a.lo) - quotient * b.lo) / b.hi);
}

/*
 * A 64-bit integer as float-float, to about 2^-48: three exact 21-bit chunks
 * summed, without the compiler's 64-bit conversion (which computes in double
 * on these cores, D56).
 */
static inline struct __libc_ff __libc_ff_from_u64(uint64_t u)
{
	struct __libc_ff sum = __libc_ff_sum((float)(uint32_t)(u >> 42) * 0x1p42f, (float)(uint32_t)((u >> 21) & 0x1fffff) * 0x1p21f);

	return __libc_ff_fast_sum(sum.hi, sum.lo + (float)(uint32_t)(u & 0x1fffff));
}

/* The same for a signed value, -2^63 included */
static inline struct __libc_ff __libc_ff_from_i64(int64_t n)
{
	struct __libc_ff magnitude = __libc_ff_from_u64(n < 0 ? 0 - (uint64_t)n : (uint64_t)n);

	return n < 0 ? (struct __libc_ff){ .hi = -magnitude.hi, .lo = -magnitude.lo } : magnitude;
}

/* pi and pi/32 as float-float */
#define __LIBC_PIF ((struct __libc_ff){ .hi = 0x1.921fb6p1f, .lo = -0x1.777a5cp-24f })
#define __LIBC_PI_32F ((struct __libc_ff){ .hi = 0x1.921fb6p-4f, .lo = -0x1.777a5cp-29f })

/* pi/2 and 1/pi as float-float */
#define __LIBC_PI_2F ((struct __libc_ff){ .hi = 0x1.921fb6p0f, .lo = -0x1.777a5cp-25f })
#define __LIBC_INV_PIF ((struct __libc_ff){ .hi = 0x1.45f306p-2f, .lo = 0x1.b9391p-27f })

/* sqrt(a.hi + a.lo) for a non-negative a, corrected by the exact residual (a - r^2)/(2r) */
static inline struct __libc_ff __libc_ff_sqrt(struct __libc_ff a)
{
	float root = sqrtf(a.hi);
	struct __libc_ff square = __libc_ff_product(root, root);

	if (root == 0)
		return (struct __libc_ff){ .hi = 0.0f, .lo = 0.0f };

	return __libc_ff_fast_sum(root, (((a.hi - square.hi) - square.lo) + a.lo) / (2.0f * root));
}

/* ln 2, 1/ln 2, ln 10, log2(10), log10(2) and 1/ln 10 as float-float */
#define __LIBC_LN2F ((struct __libc_ff){ .hi = 0x1.62e43p-1f, .lo = -0x1.05c61p-29f })
#define __LIBC_INV_LN2F ((struct __libc_ff){ .hi = 0x1.715476p0f, .lo = 0x1.4ae0cp-26f })
#define __LIBC_LN10F ((struct __libc_ff){ .hi = 0x1.26bb1cp1f, .lo = -0x1.12aabap-25f })
#define __LIBC_LOG2_10F ((struct __libc_ff){ .hi = 0x1.a934fp1f, .lo = 0x1.2f346ep-24f })
#define __LIBC_LOG10_2F ((struct __libc_ff){ .hi = 0x1.344136p-2f, .lo = -0x1.ec10cp-27f })
#define __LIBC_INV_LN10F ((struct __libc_ff){ .hi = 0x1.bcb7b2p-2f, .lo = -0x1.5b235ep-27f })

/*
 * e^r - 1 for |r| below 0.044 (Taylor to r^6; the r^7 term is below 2^-44
 * of the result), with r.hi leading so the sum keeps r's relative accuracy.
 * The r^2/2 term is formed from an exact square: rounded in float it would
 * cost 2^-36 at the top of the range (PLAN D68).
 */
static inline struct __libc_ff __libc_ff_expm1_series(struct __libc_ff r)
{
	float h = r.hi;
	struct __libc_ff square = __libc_ff_product(h, h);
	struct __libc_ff head = __libc_ff_fast_sum(h, 0.5f * square.hi);
	float tail = h * square.hi * (0x1.555556p-3f + h * (0x1.555556p-5f + h * (0x1.111112p-7f + h * 0x1.6c16c2p-10f)));

	return __libc_ff_fast_sum(head.hi, head.lo + (0.5f * square.lo + r.lo * (1.0f + h) + tail));
}

/*
 * log(1 + f) for |f| below 0.022 (Taylor to f^7; the f^8 term is below
 * 2^-46 of the result): f.hi leads and -f.hi^2/2 is added exactly (Dekker
 * square), so the rounding of the rest is below 2^-38 of the result; f.lo
 * enters through its first-order term f.lo (1 - f.hi).
 */
static inline struct __libc_ff __libc_ff_log1p_series(struct __libc_ff f)
{
	float h = f.hi;
	struct __libc_ff square = __libc_ff_product(h, h);
	struct __libc_ff head = __libc_ff_fast_sum(h, -0.5f * square.hi);
	float tail = h * square.hi * (0x1.555556p-2f + h * (-0.25f + h * (0x1.99999ap-3f + h * (-0x1.555556p-3f + h * 0x1.24924ap-3f))));

	return __libc_ff_fast_sum(head.hi, head.lo + (-0.5f * square.lo + f.lo * (1.0f - h) + tail));
}

/* x * c as float-float for a float-float constant c */
static inline struct __libc_ff __libc_ff_scale(float x, struct __libc_ff c)
{
	struct __libc_ff product = __libc_ff_product(x, c.hi);

	return __libc_ff_fast_sum(product.hi, product.lo + x * c.lo);
}

/*
 * 2^(z.hi + z.lo) = 2^*exponent * (result.hi + result.lo), to about 2^-40,
 * for |z.hi| below 2^17 (sp-nearest layer).
 */
struct __libc_ff __libc_exp2f_core(struct __libc_ff z, int *exponent);

/* 2^exponent * (v.hi + v.lo) rounded once to nearest, subnormal results included */
float __libc_exp2f_scale(struct __libc_ff v, int exponent);

/* +-2^z rounded once: +-infinity above 2^128, +-0 below 2^-151, else the exp2 core and its scaling */
float __libc_exp2f_signed(struct __libc_ff z, bool negative);

/* x = (k + d) pi/32, k returned modulo 64, *s = d pi/32 with |d| <= 1/2, for finite |x| >= 2^-6 */
int __libc_rem_pio2f(float x, struct __libc_ff *s);

/* sin(k pi/32 + s.hi + s.lo) for any k and |s| <= pi/64, to about 2^-42 */
struct __libc_ff __libc_sinf_core(int k, struct __libc_ff s);

/* atan2(y, x) for finite y and x not both zero, to about 2^-42 */
struct __libc_ff __libc_atan2f_core(struct __libc_ff y, struct __libc_ff x);

/* erf(x) for 0 <= x < 4, to about 2^-42 */
struct __libc_ff __libc_erff_core(float x);

/* log|gamma(x)| for finite x that is not a pole, *sign the sign of gamma(x), to about 2^-42 */
struct __libc_ff __libc_lgammaf_core(float x, int *sign);

/* sin y and cos y as float-float for a finite float-float y (|y.lo| at most half an ulp of y.hi), to about 2^-42 */
void __libc_sincosf_core(struct __libc_ff y, struct __libc_ff *sine, struct __libc_ff *cosine);

/* 2^exponent a b rounded once, tiny factors prescaled so that their relative accuracy survives */
float __libc_ff_round_product(struct __libc_ff a, struct __libc_ff b, int exponent);

/* sinh x and cosh x as float-float for |x| below 16, to about 2^-42 */
void __libc_sinhcoshf_core(float x, struct __libc_ff *sinh, struct __libc_ff *cosh);

/* asinh t as float-float for a finite float-float t, to about 2^-42 */
struct __libc_ff __libc_asinhf_core(struct __libc_ff t);

/* log(1 + t) as float-float for a float-float t above -1, to about 2^-42 */
struct __libc_ff __libc_log1pf_core(struct __libc_ff t);

/* 2^z m rounded once, for finite z and a sine or cosine m */
float __libc_exp2f_times(struct __libc_ff z, struct __libc_ff m);

/* e^|x| = 2^*exponent (result.hi + result.lo) and *inverse = 2^-*exponent e^-|x| (0 for exponent above 20), for |x| up to 90 */
struct __libc_ff __libc_expf_pair(float x, int *exponent, struct __libc_ff *inverse);

/* log2(x + tail) for positive finite x and |tail| at most half an ulp of x, to about 2^-40 (relative near 1) */
struct __libc_ff __libc_log2f_core(float x, float tail);

/* log2(1 + x) for finite x > -1 with |x| at least 2^-90 */
struct __libc_ff __libc_log2p1f_core(float x);

/* 2^exponent * (v.hi + v.lo) - 1 rounded once to nearest, for exponent in [-27, 128] */
float __libc_exp2m1f_scale(struct __libc_ff v, int exponent);

#endif /* _INTERNAL_MATH_H */
