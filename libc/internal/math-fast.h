/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * math-fast.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _MATH_FAST_H
#define _MATH_FAST_H

/*
 * The fast build type's libm kernels (PLAN D162, D168; docs/research/libm-fast-float-exp-log.md): accuracy below
 * the fast contract's bounds to nearest, no correction of the last bit, no rounding-mode switching.
 */

#include <stdbool.h>
#include <stdint.h>

#include "internal/math.h"

/* 2^(j/32) for j = 0..31, correctly rounded doubles, as bits (libc/math/fast/__libc_fast_exp2_table.c) */
extern const uint64_t __libc_fast_exp2_table[32];

/*
 * 2^t in double for -1022 < t < 1024 (the double-FPU cores, libc/math/fast/): t = n/32 + r with n = round(32 t) and
 * |r| <= 1/64, 2^t = 2^(n div 32) * 2^((n mod 32)/32) * 2^r with the table and a degree 3 Taylor polynomial of 2^r,
 * relative error below 2^-30.5 (the note, section 3). Rounded to float, below 0.51 ulp.
 */
static inline double __libc_fast_exp2(double t)
{
	/* round(32 t) by the 1.5 * 2^52 shift: the sum's low bits hold the integer, two's complement */
	double shifted = t * 32.0 + 0x1.8p52;
	int32_t n = (int32_t)(uint32_t)__libc_double_bits(shifted);
	double r = t - (shifted - 0x1.8p52) * 0x1p-5;
	int32_t j = n & 31;
	uint64_t scale = __libc_fast_exp2_table[j] + ((uint64_t)(int64_t)((n - j) / 32) << 52);
	double p = 1.0 + r * (0x1.62e42fefa39efp-1 + r * (0x1.ebfbdff82c58fp-3 + r * 0x1.c6b08d704a0c0p-5));

	return __libc_double_from(scale) * p;
}

/*
 * 2^t in double as __libc_fast_exp2 with the Taylor polynomial of 2^r to degree 5 (|r| <= 1/64: relative error 2^-48.7,
 * 2^-46 with t's rounding), for results that a rounding test must decide (erfcf; docs/research/libm-fast-float-trig.md
 * section 12)
 */
static inline double __libc_fast_exp2_accurate(double t)
{
	double shifted = t * 32.0 + 0x1.8p52;
	int32_t n = (int32_t)(uint32_t)__libc_double_bits(shifted);
	double r = t - (shifted - 0x1.8p52) * 0x1p-5;
	int32_t j = n & 31;
	uint64_t scale = __libc_fast_exp2_table[j] + ((uint64_t)(int64_t)((n - j) / 32) << 52);
	double p = 1.0 + r * (0x1.62e42fefa39efp-1 + r * (0x1.ebfbdff82c58fp-3 + r * (0x1.c6b08d704a0c0p-5 + r * (0x1.3b2ab6fba4e77p-7 + r * 0x1.5d87fe78a6731p-10))));

	return __libc_double_from(scale) * p;
}

/* q3(u) ~ (e^u - 1 - u - u^2/2)/u^3 of degree 4 for |u| <= ln2/2: the upper part of __libc_fast_expf_q */
static inline float __libc_fast_expf_q3(float u)
{
	float q = __builtin_fmaf(u, 0x1.a12528p-13f, 0x1.6d4362p-10f);

	q = __builtin_fmaf(u, q, 0x1.1110ep-7f);
	q = __builtin_fmaf(u, q, 0x1.5554e8p-5f);
	return __builtin_fmaf(u, q, 0x1.555556p-3f);
}

/* q(u) = 1/2 + u q3(u) ~ (e^u - 1 - u)/u^2 of degree 5 for |u| <= ln2/2, relative error 2^-30.3 on e^u (the note, section 4) */
static inline float __libc_fast_expf_q(float u)
{
	return __builtin_fmaf(u, __libc_fast_expf_q3(u), 0x1p-1f);
}

/*
 * e^(u_hi + u_lo) * 2^k in float (the single-precision FPU cores, libc/math/sp-nearest/fast/) for
 * |u_hi + u_lo| <= ln2/2 (a little more is fine), u_lo much smaller than u_hi, -153 <= k <= 130. With
 * q(u) ~ (e^u - 1 - u)/u^2 of degree 5 (relative error 2^-30.3 on e^u), e^u = 1 + (u_hi + (u_lo + u^2 q(u))): u_hi
 * enters exactly and the rest is small, so the result is below 1 ulp before scaling (the note, section 4).
 */
static inline float __libc_fast_expf_kernel(float u_hi, float u_lo, int k)
{
	float u = u_hi + u_lo;
	float p = 1.0f + (u_hi + __builtin_fmaf(u * u, __libc_fast_expf_q(u), u_lo));

	/* p in [0.7, 1.42]: 2^k by the exponent field, in two exact steps where it leaves the normal range */
	if (k >= -125 && k <= 127)
		return p * __libc_float_from((uint32_t)(k + 127) << 23);
	/* Beyond 2^127 the result overflows (exp10f's k reaches 130): 2^127 times the rest, exactly representable */
	if (k > 127)
		return p * 0x1p127f * __libc_float_from((uint32_t)k << 23);
	/* p 2^(k + 64) is exact and normal; the last multiplication rounds once into the subnormals */
	return p * __libc_float_from((uint32_t)(k + 64 + 127) << 23) * 0x1p-64f;
}

/*
 * log(1 + f) in double for 1 + f in [sqrt(1/2), sqrt(2)) (the double-FPU cores, libc/math/fast/): s = f/(2 + f),
 * exact but for the division's rounding, and log(1 + f) = 2 atanh s = 2s (1 + s^2 Q(s^2)) with the Taylor
 * coefficients 1/3 to 1/13: relative error 2^-39.5 however small f is (the note, sections 7 and 12)
 */
static inline double __libc_fast_log1p_double(double f)
{
	double s = f / (2.0 + f);
	double z = s * s;
	double q = 0x1.3b13b13b13b14p-4;

	q = q * z + 0x1.745d1745d1746p-4;
	q = q * z + 0x1.c71c71c71c71cp-4;
	q = q * z + 0x1.2492492492492p-3;
	q = q * z + 0x1.999999999999ap-3;
	q = q * z + 0x1.5555555555555p-2;

	return 2.0 * s + 2.0 * s * (z * q);
}

/* log(m) for m in [sqrt(1/2), sqrt(2)): m - 1 is exact (Sterbenz) */
static inline double __libc_fast_log_double(double m)
{
	return __libc_fast_log1p_double(m - 1.0);
}

/*
 * e^u - 1 in double for |u| <= 1/8 (the double-FPU cores' expm1f, exp2m1f, exp10m1f near 0): u (1 + u P(u)), the
 * Taylor series to u^6, relative error 2^-30.3 however small u is (the note, section 12)
 */
static inline double __libc_fast_expm1_series(double u)
{
	double p = u * 0x1.6c16c16c16c17p-10 + 0x1.1111111111111p-7;

	p = p * u + 0x1.5555555555555p-5;
	p = p * u + 0x1.5555555555555p-3;
	p = p * u + 0.5;

	/* u (1 + u P) rather than u + u^2 P, which would turn -0 into +0 */
	return u * (1.0 + u * p);
}

/* A positive normal double as 2^e m, m in [sqrt(1/2), sqrt(2)) (1 + x for the double-FPU cores' log1pf family) */
static inline double __libc_fast_log_split(double d, int *e)
{
	uint64_t bits = __libc_double_bits(d);

	*e = (int)(bits >> 52) - 1023;
	bits = (bits & 0x000fffffffffffffull) | 0x3ff0000000000000ull;
	if (bits > 0x3ff6a09e667f3bcdull) {
		bits -= 0x0010000000000000ull;
		*e += 1;
	}
	return __libc_double_from(bits);
}

/*
 * log d for a positive normal double (the double-FPU cores' hyperbolic functions; docs/research/libm-fast-float-trig.md
 * section 9): e ln2 + log m from __libc_fast_log_split, absolute error about 2^-52 beside e ln2's rounding
 */
static inline double __libc_fast_log_positive(double d)
{
	int e;
	double m = __libc_fast_log_split(d, &e);
	double l = __libc_fast_log_double(m);

	return e == 0 ? l : (double)e * 0x1.62e42fefa39efp-1 + l;
}

/* e^a in double for |a| below 709 by __libc_fast_exp2 (relative error 2^-30.5) */
static inline double __libc_fast_exp_double(double a)
{
	return __libc_fast_exp2(a * 0x1.71547652b82fep+0);
}

/* R(x) = erfc(x) e^(x^2) as degree-8 pieces (libc/math/fast/__libc_fast_erfc_table.c) */
extern const double __libc_fast_erfc_table[6][9];

/*
 * erfc a for 1/2 <= a <= 10.1 in double (the double-FPU cores' erff and erfcf; docs/research/libm-fast-float-trig.md
 * section 12): e^(-a^2) R(a), a^2 exact for a float a and the exponential relative to 2^-46
 * (__libc_fast_exp2_accurate), R from the table's pieces of width 1/2 below 3 and as v G(v^2) with v = 1/a above, each
 * to 2^-34.9: about 2^-34.5 in all
 */
static inline double __libc_fast_erfc_double(double a)
{
	const double *c;
	double t;
	double v = 1.0;
	double r;
	int i;

	if (a < 3.0) {
		i = (int)(2.0 * a) - 1;
		t = a - (0.75 + 0.5 * (double)i);
	} else {
		i = 5;
		v = 1.0 / a;
		t = v * v - 0x1.ef4397f2ba2b9p-5;
	}
	c = __libc_fast_erfc_table[i];
	r = c[8];
	for (int j = 7; j >= 0; --j)
		r = r * t + c[j];

	return __libc_fast_exp2_accurate(-(a * a) * 0x1.71547652b82fep+0) * (r * v);
}

/* erf x for |x| < 1/2 as x P(x^2), P of degree 5 fitted here (relative error 2^-36.3) */
static inline double __libc_fast_erf_small(double x)
{
	double z = x * x;

	return x * (0x1.20dd75041b4eap+0 + z * (-0x1.8127466f5b8a1p-2 + z * (0x1.ce2ef239ee029p-4 + z * (-0x1.b8203032a45abp-6 + z * (0x1.54d076df271bdp-8 + z * -0x1.936a4a7830bd8p-11)))));
}

/* A positive finite float as 2^e m, m in [sqrt(1/2), sqrt(2)), in double (subnormals first scaled by 2^23) */
static inline double __libc_fast_logf_split(float x, int *e)
{
	uint32_t bits = __libc_float_bits(x);
	int bias = 0;

	if (bits < 0x00800000) {
		bits = __libc_float_bits(x * 0x1p23f);
		bias = 23;
	}
	/* m in [1, 2), then halved above sqrt(2) */
	*e = __libc_float_exponent(bits) - 127 - bias;
	bits = (bits & 0x007fffff) | 0x3f800000;
	if (bits > 0x3fb504f3) {
		bits -= 0x00800000;
		*e += 1;
	}
	return (double)__libc_float_from(bits);
}

/*
 * The single-precision log reduction (libc/math/sp-nearest/fast/; the note, section 8): a positive finite x is
 * 2^k z with z in [0.6992, 1.3984), the 2^18-ulp step of z's bits from 0x3f330000 giving entry i of 32, and
 * r = z (1/c_i) - 1 by a fused multiply-add, |r| < 0.0235; within 2^-5 of 1, r = x - 1 exactly against the entry holding 1, c = 1.
 * The tables: 1/c_i, and -log(1/c_i) in each base as a high part (a multiple of 2^-16, so that k ln2_hi + high is
 * exact) and a low part.
 */
extern const float __libc_fast_logf_invc[32];
extern const float __libc_fast_logf_ln[32][2];
extern const float __libc_fast_logf_log2[32][2];
extern const float __libc_fast_logf_log10[32][2];

/* z, k and i of __libc_fast_logf_reduce without its near-1 path: powf forms r and its error itself */
static inline float __libc_fast_logf_split_z(float x, int *k, int *i)
{
	uint32_t bits = __libc_float_bits(x);
	int bias = 0;
	uint32_t offset;

	if (bits < 0x00800000) {
		bits = __libc_float_bits(x * 0x1p23f);
		bias = 23;
	}
	offset = bits - 0x3f330000;
	*i = (int)((offset >> 18) & 31);
	/* The exponent field of the offset, signed: an exact division of a multiple of 2^23 */
	*k = (int32_t)(offset & 0xff800000) / 0x800000 - bias;

	return __libc_float_from(bits - (offset & 0xff800000));
}

static inline float __libc_fast_logf_reduce(float x, int *k, int *i)
{
	uint32_t bits = __libc_float_bits(x);
	int bias = 0;
	uint32_t offset;

	/*
	 * Within 2^-5 of 1, r = x - 1 exactly against the entry holding 1: next to it log c_i and r would cancel and
	 * leave their roundings at up to 2.7 ulp of the result (measured); beyond, |log x| > 0.031 and they cost 1/8 ulp
	 */
	if (bits - 0x3f780000u < 0x3f840000u - 0x3f780000u) {
		*k = 0;
		*i = 19;
		return x - 1.0f;
	}
	if (bits < 0x00800000) {
		bits = __libc_float_bits(x * 0x1p23f);
		bias = 23;
	}
	offset = bits - 0x3f330000;
	*i = (int)((offset >> 18) & 31);
	/* The exponent field of the offset, signed: an exact division of a multiple of 2^23 */
	*k = (int32_t)(offset & 0xff800000) / 0x800000 - bias;

	return __builtin_fmaf(__libc_float_from(bits - (offset & 0xff800000)), __libc_fast_logf_invc[*i], -1.0f);
}

/* log1p(r) - r = r^2 q(r) for |r| < 2^-5: q of degree 3, relative error 2^-30.0 on log1p(r) */
static inline float __libc_fast_log1pf_tail(float r)
{
	float q = __builtin_fmaf(r, 0x1.99e444p-3f, -0x1.002b8ep-2f);

	q = __builtin_fmaf(r, q, 0x1.555554p-2f);
	q = __builtin_fmaf(r, q, -0x1.fffffep-2f);

	return r * r * q;
}

/* A float's integer kind for powf: 1 odd, 0 even (every |y| from 2^24 up), -1 not an integer */
static inline int __libc_fast_powf_kind(float y)
{
	if (truncf(y) != y)
		return -1;
	if (fabsf(y) >= 0x1p24f)
		return 0;
	return ((int32_t)y & 1) != 0 ? 1 : 0;
}

/*
 * powf's special cases for every fast powf (C23 7.12.7.5, F.10.4.5): true with *result when they decide, else
 * false with *negative, the sign of |x|^y (a negative x to an odd integer power), for the caller to compute
 */
static inline bool __libc_fast_powf_special(float x, float y, float *result, bool *negative)
{
	int kind;

	if (y == 0 || x == 1.0f) {
		*result = 1.0f;
		return true;
	}
	if (isnan(x) || isnan(y)) {
		*result = x + y;
		return true;
	}
	if (isinf(y)) {
		*result = fabsf(x) == 1.0f ? 1.0f : (fabsf(x) < 1.0f) == (y < 0) ? INFINITY :
																		   0.0f;
		return true;
	}
	kind = __libc_fast_powf_kind(y);
	*negative = signbit(x) && kind == 1;
	if (x == 0) {
		/* +-infinity with divide-by-zero for a negative y: a division at run time */
		*result = y < 0 ? (*negative ? -1.0f : 1.0f) / fabsf(x) : *negative ? -0.0f :
																			  0.0f;
		return true;
	}
	if (isinf(x)) {
		*result = y < 0 ? 0.0f : INFINITY;
		if (*negative)
			*result = -*result;
		return true;
	}
	if (x < 0 && kind < 0) {
		*result = __libc_math_invalidf();
		return true;
	}
	if (y == 1.0f) {
		*result = x;
		return true;
	}
	return false;
}

/* a + b as a float and its exact error (TwoSum, either order) */
static inline float __libc_fast_two_sum(float a, float b, float *error)
{
	float s = a + b;
	float bb = s - a;

	*error = (a - (s - bb)) + (b - bb);
	return s;
}

/*
 * log2 x for a positive finite float as hi + *lo, to about 2^-35 relative (powf on the single-precision FPU
 * cores; the note, section 10): r = z/c_i - 1 exact as (p - 1) + p_lo with p = z (1/c_i) and p_lo its error
 * by a fused multiply-add; log1p(r) = r - r^2/2 + r^3 q3(r) with the square's error kept and q3 of degree 3
 * (2^-37.3); the products by 1/ln2 with their errors; k + log2 c_i's high part exact; two exact sums
 */
static inline float __libc_fast_log2f_extended(float x, float *lo)
{
	int k;
	int i;
	float z = __libc_fast_logf_split_z(x, &k, &i);
	const float *entry = __libc_fast_logf_log2[i];
	float invc = __libc_fast_logf_invc[i];
	float p = z * invc;
	float p_lo = __builtin_fmaf(z, invc, -p);
	float r = p - 1.0f;
	float r2 = r * r;
	float r2_lo = __builtin_fmaf(r, r, -r2);
	float b = -0.5f * r2;
	float a_hi = r * 0x1.715476p+0f;
	float a_lo = __builtin_fmaf(r, 0x1.715476p+0f, -a_hi);
	float b_hi = b * 0x1.715476p+0f;
	float b_lo = __builtin_fmaf(b, 0x1.715476p+0f, -b_hi);
	float q = __builtin_fmaf(r, -0x1.50767ep-3f, 0x1.99ae16p-3f);
	float rest;
	float c_hi;
	float c_lo;
	float e1;
	float e2;
	float e3;
	float hi;

	q = __builtin_fmaf(r, q, -0x1.00001cp-2f);
	q = __builtin_fmaf(r, q, 0x1.555556p-2f);
	/*
	 * the terms of order r^3 and below r's rounding: p_lo/(1 + r) as p_lo (1 - r + r^2) (p_lo reaches 2^-24, and
	 * without r^2 it left 2^-38 next to 1, 1.38 ulp of x^(2^14), measured), the square's error, r^3 q3
	 */
	rest = __builtin_fmaf(p_lo, __builtin_fmaf(r, r, 1.0f - r), __builtin_fmaf(-0.5f, r2_lo, r * r2 * q));
	rest = a_lo + b_lo + __builtin_fmaf(r + b, 0x1.4ae0c0p-26f, rest * 0x1.715476p+0f);

	/*
	 * log2 c_i as a full float and its remainder (Fast2Sum of the table's parts, exact): the 2^-16 split that keeps
	 * the logarithms' high sum exact would leave a low part up to 2^-17, and its roundings at 2^-42 cost
	 * x near 1 to the power 2^14 up to 1.44 ulp (measured); then k and the terms by exact sums
	 */
	c_hi = entry[0] + entry[1];
	c_lo = (entry[0] - c_hi) + entry[1];
	hi = __libc_fast_two_sum((float)k, c_hi, &e1);
	hi = __libc_fast_two_sum(hi, a_hi, &e2);
	hi = __libc_fast_two_sum(hi, b_hi, &e3);
	*lo = ((e1 + e2) + e3) + (c_lo + rest);
	return hi;
}

/*
 * e^(u_hi + u_lo) 2^k - 1 for |k| <= 24 as the unrounded pair return + *lo, with q3 = __libc_fast_expf_q3(u): the
 * leading terms of __libc_fast_expm1f_kernel by exact two-sums (its comment below), so that only the caller's last
 * addition rounds much (tanhf near 0)
 */
static inline float __libc_fast_expm1f_pair(float u_hi, float u_lo, int k, float q3, float *lo)
{
	float square = u_hi * u_hi;
	float rest = __builtin_fmaf(square * u_hi, q3, __builtin_fmaf(u_lo, 1.0f + u_hi, 0.5f * __builtin_fmaf(u_hi, u_hi, -square)));
	float s;
	float hi;
	float error;
	float error2;

	if (k == 0) {
		hi = __libc_fast_two_sum(u_hi, 0.5f * square, &error);
		*lo = error + rest;
		return hi;
	}
	s = __libc_float_from((uint32_t)(k + 127) << 23);
	hi = __libc_fast_two_sum(s - 1.0f, s * u_hi, &error);
	hi = __libc_fast_two_sum(hi, s * (0.5f * square), &error2);
	*lo = (error + error2) + s * rest;
	return hi;
}

/*
 * e^(u_hi + u_lo) 2^k - 1 in float (the single-precision FPU cores' expm1f, exp2m1f, exp10m1f), with the
 * arguments of __libc_fast_expf_kernel and -27 <= k <= 130 and the kernel's polynomial: for |k| <= 24 the leading
 * terms by exact two-sums (below); for k < -24, 2^k e^u is below 2^-24 beside -1; above 2^24, the 1 joins the tail
 * as 2^-k and the scaling is the kernel's (the note, section 12).
 */
static inline float __libc_fast_expm1f_kernel(float u_hi, float u_lo, int k)
{
	float u = u_hi + u_lo;
	/* One call site, so that -Os inlines it */
	float q3 = __libc_fast_expf_q3(u);
	float tail;
	float square;
	float rest;
	float s;
	float hi;
	float error;
	float error2;

	if (k >= -24 && k <= 24) {
		/*
		 * e^u - 1 = u_hi + u_hi^2/2 + (u_hi^2/2's error + u_lo (1 + u_hi) + u_hi^3 q3(u)), the square from u_hi exactly
		 * (a fused multiply-add for its error): squaring u rounded cost a quarter ulp of a result near 0.24 and the
		 * whole tail rounded at once 2^k times its rounding, 1.12 ulp in all (exp2m1f, measured). For k = 0 that is
		 * the result, accurate relative to |u| however small; otherwise 2^k e^u - 1 = (2^k - 1) + 2^k u_hi +
		 * 2^k u_hi^2/2 by exact two-sums, then 2^k times the rest. __libc_fast_expm1f_pair is the same as a pair; the
		 * kernel keeps its own copy, as sharing it cost the m1 forms 0.8 % (P11.04c, measured)
		 */
		square = u_hi * u_hi;
		rest = __builtin_fmaf(square * u_hi, q3, __builtin_fmaf(u_lo, 1.0f + u_hi, 0.5f * __builtin_fmaf(u_hi, u_hi, -square)));
		if (k == 0) {
			hi = __libc_fast_two_sum(u_hi, 0.5f * square, &error);
			return hi + (error + rest);
		}
		s = __libc_float_from((uint32_t)(k + 127) << 23);
		hi = __libc_fast_two_sum(s - 1.0f, s * u_hi, &error);
		hi = __libc_fast_two_sum(hi, s * (0.5f * square), &error2);
		return hi + ((error + error2) + s * rest);
	}
	/* Beside -1 and beyond 2^24 the result is far from 0: the tail rounded at once is enough */
	tail = __builtin_fmaf(u * u, __builtin_fmaf(u, q3, 0x1p-1f), u_lo);
	if (k < -24)
		return -1.0f + __libc_float_from((uint32_t)(k + 127) << 23) * (1.0f + (u_hi + tail));
	if (k <= 127)
		return (1.0f + (u_hi + (tail - __libc_float_from((uint32_t)(127 - k) << 23)))) * __libc_float_from((uint32_t)(k + 127) << 23);
	/* exp10m1f's k reaches 130: 2^127 times the rest, which overflows */
	return (1.0f + (u_hi + tail)) * 0x1p127f * __libc_float_from((uint32_t)k << 23);
}

/*
 * The log1p reduction (the single-precision FPU cores' log1pf, log2p1f, log10p1f; the note, section 12): for
 * |x| < 2^-5, r = x exactly against the entry holding 1 (k = 0, c = 1); otherwise 1 + x = u + c exactly (a two-sum),
 * u = 2^k z reduced as __libc_fast_logf_reduce does, r = z/c_i - 1 by a fused multiply-add, and *lo = c/(1 + x) as
 * c 2^-k (1/c_i)(1 - r), so that log(1 + x) = k ln2 + log c_i + log1p(r) + *lo to second order
 */
static inline float __libc_fast_log1pf_reduce(float x, int *k, int *i, float *lo)
{
	float c;
	float u;
	float z;
	float invc;
	float r;
	float w;

	if (__builtin_fabsf(x) < 0x1p-5f) {
		*k = 0;
		*i = 19;
		*lo = 0.0f;
		return x;
	}
	u = __libc_fast_two_sum(1.0f, x, &c);
	z = __libc_fast_logf_split_z(u, k, i);
	invc = __libc_fast_logf_invc[*i];
	r = __builtin_fmaf(z, invc, -1.0f);
	/* 2^-k for k <= 100; beyond, |c| <= 1 and c/u is far below the result's ulp either way */
	w = c * __libc_float_from((uint32_t)(127 - (*k < 100 ? *k : 100)) << 23) * invc;
	*lo = __builtin_fmaf(w, -r, w);
	return r;
}

/*
 * x = k pi/2 + r for |x| < 2^20 in double (the double-FPU cores' sinf family, docs/research/libm-fast-float-trig.md
 * section 3): k = round(x 2/pi), r = ((x - k P1) - k P2) - k P3 with P1 and P2 of 33 bits, so that k P1 and k P2 are exact
 * for |k| < 2^20 and x - k P1 is exact too (both multiples of 2^-32 for |x| >= 2^-9, the difference below 2^21); r's error
 * is about 2^-52 of r however close x is to a multiple of pi/2
 */
static inline int __libc_fast_rem_pio2(double x, double *r)
{
	double shifted = x * 0x1.45f306dc9c883p-1 + 0x1.8p52;
	int k = (int)(uint32_t)__libc_double_bits(shifted);
	double kd = shifted - 0x1.8p52;

	*r = ((x - kd * 0x1.921fb544p+0) - kd * 0x1.0b4611a6p-34) - kd * 0x1.3198a2e037073p-69;
	return k;
}

/* sin r for |r| <= pi/4 in double: r + r z S(z), z = r^2, the Taylor series to r^11 (relative error 2^-36.7) */
static inline double __libc_fast_sin_double(double r)
{
	double z = r * r;
	double p = z * -0x1.ae64567f544e4p-26 + 0x1.71de3a556c734p-19;

	p = p * z - 0x1.a01a01a01a01ap-13;
	p = p * z + 0x1.1111111111111p-7;
	p = p * z - 0x1.5555555555555p-3;

	return r + r * (z * p);
}

/* cos r for |r| <= pi/4 in double: 1 + z C(z), z = r^2, the Taylor series to r^12 (error 2^-38) */
static inline double __libc_fast_cos_double(double r)
{
	double z = r * r;
	double p = z * 0x1.1eed8eff8d898p-29 - 0x1.27e4fb7789f5cp-22;

	p = p * z + 0x1.a01a01a01a01ap-16;
	p = p * z - 0x1.6c16c16c16c17p-10;
	p = p * z + 0x1.5555555555555p-5;
	p = p * z - 0.5;

	return 1.0 + z * p;
}

/* sin(k pi/2 + r) for |r| <= pi/4: the quadrant picks sin or cos and the sign */
static inline double __libc_fast_sin_quadrant(int k, double r)
{
	double value = (k & 1) == 0 ? __libc_fast_sin_double(r) : __libc_fast_cos_double(r);

	return (k & 2) == 0 ? value : -value;
}

/*
 * x = k pi/32 + s for 2^-6 <= |x| < 64 in float (the single-precision FPU cores' sinf family; the note, section 4), a
 * Cody-Waite reduction in four pieces: P1, P2, P3 of 14 bits, so that k P1, k P2, k P3 are exact (|k| < 2^10), x - k P1
 * exact by a fused multiply-add (both multiples of 2^-28, the difference below 0.052), the next two by exact two-sums, and
 * P4 rounded (k P4 below 2^-36); s as s_hi + *s_lo to about 2^-59 absolute. Near the multiples of pi/2, where sin or cos is
 * small, |s| is at least 2^-26.3 for these floats (measured over all of them), so that is 2^-33 relative
 */
static inline int __libc_fast_rem_pio32f(float x, float *s_lo, float *s_hi)
{
	float k = __builtin_fmaf(x, 0x1.45f306p+3f, 0x1.8p23f) - 0x1.8p23f;
	float e1;
	float e2;
	float hi = __libc_fast_two_sum(__builtin_fmaf(-k, 0x1.9218p-4f, x), -k * 0x1.ed5p-18f, &e1);
	float tail;

	hi = __libc_fast_two_sum(hi, -k * 0x1.10bp-34f, &e2);
	tail = __builtin_fmaf(-k, 0x1.184698p-48f, e1 + e2);
	*s_hi = hi + tail;
	*s_lo = (hi - *s_hi) + tail;
	return (int)k;
}

/* sin and cos of j pi/32 as float hi and lo, j = 0 .. 8 (libc/math/sp-nearest/fast/__libc_fast_sinf_table.c) */
extern const float __libc_fast_sinf_table[9][4];

/*
 * sin(k pi/32 + s) for any k (taken modulo 64) and s = s_hi + s_lo with |s| <= pi/64, float only (the single-precision FPU
 * cores, the note, section 4), as the unrounded pair return + *lo: k + 8 = 16 q + (j + 8) gives the quadrant q and the
 * table angle a = |j| pi/32; with f and g the table's sin a and cos a (signed, swapped for odd q), the value is
 * f + g s_hi + (f_lo + f (cos s - 1) + g (sin s - s_hi) + g_lo s_hi), the first product exact by a fused multiply-add and
 * the leading sum by a two-sum, so only the caller's last addition rounds much; sin s - s = s^3 (-1/6 + s^2/120) and
 * cos s - 1 = s^2 (-1/2 + s^2/24), their next terms below 2^-31 of the result
 */
static inline float __libc_fast_sinf_kernel(int k, float s_hi, float s_lo, float *lo)
{
	int quadrant = ((k + 8) & 63) >> 4;
	int j = ((k + 8) & 15) - 8;
	const float *t = __libc_fast_sinf_table[j < 0 ? -j : j];
	float sign = j < 0 ? -1.0f : 1.0f;
	float z = s_hi * s_hi;
	float sine = __builtin_fmaf(s_hi * z, __builtin_fmaf(z, 0x1.111112p-7f, -0x1.555556p-3f), s_lo);
	float cosine = z * __builtin_fmaf(z, 0x1.555556p-5f, -0.5f);
	float f_hi;
	float f_lo;
	float g_hi;
	float g_lo;
	float p;
	float hi;
	float error;

	if ((quadrant & 1) == 0) {
		f_hi = sign * t[0];
		f_lo = sign * t[1];
		g_hi = t[2];
		g_lo = t[3];
	} else {
		f_hi = t[2];
		f_lo = t[3];
		g_hi = -sign * t[0];
		g_lo = -sign * t[1];
	}
	p = g_hi * s_hi;
	hi = __libc_fast_two_sum(f_hi, p, &error);
	*lo = error + (__builtin_fmaf(g_hi, s_hi, -p) + (f_lo + __builtin_fmaf(f_hi, cosine, __builtin_fmaf(g_hi, sine, g_lo * s_hi))));
	if (quadrant >= 2) {
		*lo = -*lo;
		return -hi;
	}
	return hi;
}

/*
 * (a + a_lo) / (b + b_lo) for the kernel's pairs: each normalised first by a Fast2Sum (exact, |lo| below 2^-10 of hi: the
 * kernel's lo carries the polynomial terms, and dividing the remainder by b rather than b + b_lo cost 2^-22, 4.5 ulp of
 * tanf, measured), then q + (a - q b + a_lo - q b_lo) / b with a - q b exact by a fused multiply-add
 */
static inline float __libc_fast_divide_pairs(float a, float a_lo, float b, float b_lo)
{
	float a_hi = a + a_lo;
	float b_hi = b + b_lo;
	float q;

	a_lo = (a - a_hi) + a_lo;
	b_lo = (b - b_hi) + b_lo;
	q = a_hi / b_hi;

	return q + (__builtin_fmaf(-q, b_hi, a_hi) + __builtin_fmaf(-q, b_lo, a_lo)) / b_hi;
}

/* atan(j/8) for j = 0 .. 8, correctly rounded doubles (libc/math/fast/__libc_fast_atan_table.c) */
extern const double __libc_fast_atan_table[9];

/*
 * atan a for a >= 0 (or +infinity) in double (the double-FPU cores' inverse trigonometric functions; the trigonometric note,
 * section 6): above 1, pi/2 - atan(1/a); then a = c + d against the nearest c = j/8, t = (a - c)/(1 + a c) with a - c exact
 * (Sterbenz), |t| <= 1/16, atan a = atan c + atan t with atan t = t (1 - t^2/3 + ... + t^8/9), relative error 2^-43.5;
 * t = a itself for j = 0, so the relative accuracy holds near 0
 */
static inline double __libc_fast_atan_double(double a)
{
	bool invert = a > 1.0;
	int j;
	double c;
	double t;
	double z;
	double r;

	if (invert)
		a = 1.0 / a;
	/*
	 * a 8 + 1/2 rounds up to 1 only within 2^-57 below 1/16, where a - 1/8 is then inexact by 2^-53 of it: harmless here,
	 * unlike the float kernel below
	 */
	j = (int)(a * 8.0 + 0.5);
	c = (double)j * 0.125;
	t = (a - c) / (1.0 + a * c);
	z = t * t;
	r = __libc_fast_atan_table[j] + (t + t * (z * (-0x1.5555555555555p-2 + z * (0x1.999999999999ap-3 + z * (-0x1.2492492492492p-3 + z * 0x1.c71c71c71c71cp-4)))));

	return invert ? 0x1.921fb54442d18p+0 - r : r;
}

/* atan(j/8) as float and remainder, j = 0 .. 8 (libc/math/sp-nearest/fast/__libc_fast_atanf_table.c) */
extern const float __libc_fast_atanf_table[9][2];

/*
 * atan(a + a_lo) for 0 <= a <= 1 in float (the single-precision FPU cores' inverse trigonometric functions; the
 * trigonometric note, section 7), as the unrounded pair return + *lo: a = c + d against the nearest c = j/8,
 * t = (a - c + a_lo)/(1 + a c) as q + q_lo (a - c exact by Sterbenz, as c/2 <= a <= 2c for j >= 1, the denominator's rounding error by a fused multiply-add, the
 * quotient's remainder by another), atan t = t - t^3/3 + t^5/5 - t^7/7 (|t| <= 1/16, relative error 2^-35), and
 * atan c + q by a two-sum, so only the caller's last addition rounds much; for j = 0, t is a itself
 */
static inline float __libc_fast_atanf_kernel(float a, float a_lo, float *lo)
{
	/* The nearest j/8 from a 16 (exact) truncated: a * 8 + 0.5 rounds up to 1 just below 1/16, and a - 1/8 is then inexact */
	int j = ((int)(a * 16.0f) + 1) >> 1;
	float c = (float)j * 0.125f;
	float n = a - c;
	float d = __builtin_fmaf(a, c, 1.0f);
	float d_lo = __builtin_fmaf(a, c, 1.0f - d) + a_lo * c;
	float q = n / d;
	float q_lo = (__builtin_fmaf(-q, d, n) + __builtin_fmaf(-q, d_lo, a_lo)) / d;
	float z = q * q;
	float tail = q * z * __builtin_fmaf(z, __builtin_fmaf(z, -0x1.24924ap-3f, 0x1.99999ap-3f), -0x1.555556p-2f);
	float error;
	float hi = __libc_fast_two_sum(__libc_fast_atanf_table[j][0], q, &error);

	*lo = error + (__libc_fast_atanf_table[j][1] + (q_lo + tail));
	return hi;
}

/* (c_hi + c_lo) - (hi + lo) as an unrounded pair, the leading difference by a two-sum */
static inline float __libc_fast_pair_from(float c_hi, float c_lo, float hi, float lo, float *out_lo)
{
	float error;
	float sum = __libc_fast_two_sum(c_hi, -hi, &error);

	*out_lo = error + (c_lo - lo);
	return sum;
}

/* (hi + lo) / pi rounded: the product by 1/pi as float and remainder, the leading product's error by a fused multiply-add */
static inline float __libc_fast_pair_over_pi(float hi, float lo)
{
	float p = hi * 0x1.45f306p-2f;

	return p + (__builtin_fmaf(hi, 0x1.45f306p-2f, -p) + __builtin_fmaf(hi, 0x1.b93910p-27f, lo * 0x1.45f306p-2f));
}

/*
 * asin(x) for |x| <= 1/2 as x + x^3 P(x^2), the unrounded pair x + *lo: P of degree 6 fitted here by Chebyshev
 * interpolation on [0, 0.25] (relative error 2^-29.9 on asin); with s from sqrt((1 - |x|)/2) the same gives the half-angle
 * forms of asin and acos above 1/2
 */
static inline float __libc_fast_asinf_small(float x, float x_lo, float *lo)
{
	float z = x * x;
	float p = __builtin_fmaf(z, 0x1.fbaa70p-6f, 0x1.5a41fcp-7f);

	p = __builtin_fmaf(z, p, 0x1.82e318p-6f);
	p = __builtin_fmaf(z, p, 0x1.efed0cp-6f);
	p = __builtin_fmaf(z, p, 0x1.6dc0f6p-5f);
	p = __builtin_fmaf(z, p, 0x1.33331ep-4f);
	p = __builtin_fmaf(z, p, 0x1.555556p-3f);
	*lo = __builtin_fmaf(x * z, p, x_lo);
	return x;
}

/* sqrt((1 - a)/2) for 1/2 < a <= 1 as s + *lo: 1 - a and the halving exact, the square root's remainder by a fused multiply-add */
static inline float __libc_fast_half_angle(float a, float *lo)
{
	float z = (1.0f - a) * 0.5f;
	float s = __builtin_sqrtf(z);

	*lo = s > 0 ? __builtin_fmaf(-s, s, z) / (2.0f * s) : 0.0f;
	return s;
}

/*
 * e^a = 2^k (return + *lo) for 0 <= a <= 90 in float (the single-precision FPU cores' hyperbolic functions; the
 * trigonometric note, section 10): a = k ln2 + u as expf (u_hi by a fused multiply-add, u_lo = k times ln2's low part),
 * and e^u = 1 + u_hi + (u_lo + u^2 q(u)) with 1 + u_hi by a two-sum, then normalised by a Fast2Sum (the tail reaches
 * 0.06, and a reciprocal 1/hi - lo/hi^2 of the raw pair measured 5e-4): an unrounded pair in [0.7, 1.42] to about 2^-30
 */
static inline float __libc_fast_expf_pair(float a, int *k, float *lo)
{
	float kf = __builtin_fmaf(a, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;
	float u_hi = __builtin_fmaf(-kf, 0x1.62e430p-1f, a);
	float u_lo = kf * 0x1.05c610p-29f;
	float u = u_hi + u_lo;
	float error;
	float hi = __libc_fast_two_sum(1.0f, u_hi, &error);

	float tail = error + __builtin_fmaf(u * u, __libc_fast_expf_q(u), u_lo);
	float sum = hi + tail;

	*k = (int)kf;
	*lo = tail - (sum - hi);
	return sum;
}

/*
 * log1p(u + u_lo) for u > -1 and |u_lo| at most half an ulp of u, rounded once (the single-precision FPU cores' asinhf,
 * acoshf, atanhf; the note, section 10): the log1pf reduction of u, with u_lo/(1 + u) beside its own correction
 */
static inline float __libc_fast_log1pf_pair(float u, float u_lo)
{
	const float *entry;
	float kf;
	float r;
	float lo;
	int k;
	int i;

	r = __libc_fast_log1pf_reduce(u, &k, &i, &lo);
	entry = __libc_fast_logf_ln[i];
	kf = (float)k;

	return __builtin_fmaf(kf, 0x1.62e400p-1f, entry[0]) + (r + (__builtin_fmaf(kf, 0x1.7f7d1cp-20f, entry[1]) + (__libc_fast_log1pf_tail(r) + (lo + u_lo / (1.0f + u)))));
}

/* log 2a for a finite a from 2 up, rounded once: the logf reduction of a with k + 1 (asinhf and acoshf above 2^12) */
static inline float __libc_fast_logf_twice(float a)
{
	const float *entry;
	float kf;
	float r;
	int k;
	int i;

	r = __libc_fast_logf_reduce(a, &k, &i);
	entry = __libc_fast_logf_ln[i];
	kf = (float)(k + 1);

	return __builtin_fmaf(kf, 0x1.62e400p-1f, entry[0]) + (r + (__builtin_fmaf(kf, 0x1.7f7d1cp-20f, entry[1]) + __libc_fast_log1pf_tail(r)));
}

/* sqrt(w + w_lo) for w > 0 as the pair return + *lo, the square root's remainder by a fused multiply-add */
static inline float __libc_fast_sqrtf_pair(float w, float w_lo, float *lo)
{
	float s = __builtin_sqrtf(w);

	*lo = (__builtin_fmaf(-s, s, w) + w_lo) / (2.0f * s);
	return s;
}

/* e^(a + a_lo) = 2^k (return + *lo) for -104 <= a <= 90 and |a_lo| at most half an ulp of a: __libc_fast_expf_pair with the argument's low part */
static inline float __libc_fast_expf_pair_lo(float a, float a_lo, int *k, float *lo)
{
	float kf = __builtin_fmaf(a, 0x1.715476p+0f, 0x1.8p23f) - 0x1.8p23f;
	float u_hi = __builtin_fmaf(-kf, 0x1.62e430p-1f, a);
	float u_lo = __builtin_fmaf(kf, 0x1.05c610p-29f, a_lo);
	float u = u_hi + u_lo;
	float error;
	float hi = __libc_fast_two_sum(1.0f, u_hi, &error);
	float tail = error + __builtin_fmaf(u * u, __libc_fast_expf_q(u), u_lo);
	float sum = hi + tail;

	*k = (int)kf;
	*lo = tail - (sum - hi);
	return sum;
}

/* R(x) = erfc(x) e^(x^2) as degree-7 float pieces (libc/math/sp-nearest/fast/__libc_fast_erfcf_table.c) */
extern const float __libc_fast_erfcf_table[6][9];

/*
 * erfc a = 2^k (return + *lo) for 1/2 <= a <= 10.1 in float (the single-precision FPU cores' erff and erfcf; the
 * trigonometric note, section 13): e^(-a^2) from a^2 as an exact pair (__libc_fast_expf_pair_lo; -a^2 - k ln2 is exact,
 * k ln2's high part having 21 bits), R from the table as c0 + c1 t + t^2 Q(t) with c0 a pair and c0 + c1 t exact, above 3
 * as v G(v^2) with v = 1/a and its remainder; the two pairs multiplied with the leading product's error by a fused
 * multiply-add
 */
static inline float __libc_fast_erfcf_kernel(float a, int *k, float *lo)
{
	float z = a * a;
	float z_lo = __builtin_fmaf(a, a, -z);
	float e_lo;
	float e = __libc_fast_expf_pair_lo(-z, -z_lo, k, &e_lo);
	const float *c;
	float t;
	float q;
	float r;
	float r_lo;
	float v = 1.0f;
	float v_lo;
	float m;
	int i;

	if (a < 3.0f) {
		i = (int)(2.0f * a) - 1;
		t = a - (0.75f + 0.5f * (float)i);
	} else {
		i = 5;
		v = 1.0f / a;
		t = v * v - 0x1.ef4398p-5f;
	}
	c = __libc_fast_erfcf_table[i];
	q = __builtin_fmaf(c[8], t, c[7]);
	q = __builtin_fmaf(q, t, c[6]);
	q = __builtin_fmaf(q, t, c[5]);
	q = __builtin_fmaf(q, t, c[4]);
	q = __builtin_fmaf(q, t, c[3]);
	/*
	 * c0 + c1 t exactly (the product's error by a fused multiply-add, the sum by a two-sum): c1 t reaches a third of R,
	 * and rounding it with the rest measured 1.06 ulp of erfcf; t^2 Q is below a tenth of R
	 */
	m = c[2] * t;
	r = __libc_fast_two_sum(c[0], m, &r_lo);
	r_lo = r_lo + (c[1] + __builtin_fmaf(c[2], t, -m) + t * t * q);
	if (i == 5) {
		/* R = v G(w), v's remainder by a fused multiply-add */
		v_lo = __builtin_fmaf(-v, a, 1.0f) / a;
		m = v * r;
		r_lo = __builtin_fmaf(v, r, -m) + __builtin_fmaf(v, r_lo, v_lo * r);
		r = m;
	}
	m = e * r;
	*lo = __builtin_fmaf(e, r, -m) + __builtin_fmaf(e, r_lo, e_lo * r);
	return m;
}

/* erf x / x - 2/sqrt(pi) over x^2 for |x| < 0.875, degree 5 in z = x^2 fitted here (the series' relative error 2^-31.2) */
static inline float __libc_fast_erff_tail(float z)
{
	float q = __builtin_fmaf(z, 0x1.67e2bap-14f, -0x1.ad4c58p-11f);

	q = __builtin_fmaf(z, q, 0x1.5517b4p-8f);
	q = __builtin_fmaf(z, q, -0x1.b815eep-6f);
	q = __builtin_fmaf(z, q, 0x1.ce2e56p-4f);
	return __builtin_fmaf(z, q, -0x1.812744p-2f);
}

#endif /* _MATH_FAST_H */
