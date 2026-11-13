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

/* sin(j pi/32) for j = 0..16 as exact pairs; cos(j pi/32) is entry 16 - j (libc/math/fast/__libc_fast_sin_table.c, D196) */
extern const struct __libc_dd __libc_fast_sin_table[17];

/*
 * The double sin, cos, sincos and tan of the double-FPU cores (PLAN D196; docs/research/libm-fast-double.md, section 14):
 * the strict reduction and double-double cores for |x| >= 2^20 and next to a zero, out of line
 * (libc/math/fast/__libc_fast_trig_slow.c), the result itself so that the callers end in a tail call: kind 0 for sin, 1
 * for cos, 2 for tan
 */
double __libc_fast_trig_slow(double x, int kind);

/*
 * The float functions' path from 2^20 (sinf, cosf, sincosf, tanf, D175): NaN and the infinities as the double functions
 * treat them, else the slow path itself, so that they give the strict double results without pulling the fast double files
 */
static inline double __libc_fast_trig_large(float x, int kind)
{
	if (isnan(x))
		return (double)x + (double)x;
	if (isinf(x))
		return __libc_math_invalid();

	return __libc_fast_trig_slow((double)x, kind);
}

/*
 * a = n pi/32 + t + t_lo for 0 <= a < 2^20, |t| <= pi/64: Cody-Waite with pi/32 in three parts, P1 and P2 of 29 bits (so
 * n P1 and n P2 are exact for n < 2^24), a - n P1 exact (Sterbenz), the next difference exact by a fast two-difference
 * (when |a - n P1| < |n P2| the difference is exact itself), then n P3 rounded into the low part, normalized: t + t_lo
 * to about 2^-91
 */
static inline uint32_t __libc_fast_rem_pio32(double a, double *t, double *t_lo)
{
	double shifted = a * 0x1.45f306dc9c883p+3 + 0x1.8p52;
	uint32_t n = (uint32_t)__libc_double_bits(shifted);
	double nd = shifted - 0x1.8p52;
	double u = a - nd * 0x1.921fb54p-4;
	double w = nd * 0x1.10b4611p-34;
	double h = u - w;
	double l = ((u - h) - w) - nd * 0x1.4c4c6628b80dcp-63;

	/* Normalized: l reaches 2^-40, and the kernel takes t_lo to first order only (t_lo t^2/2 would be 2^-49) */
	*t = h + l;
	*t_lo = l - (*t - h);
	return n;
}

/*
 * Whether a = n pi/32 + t lies next to a zero of sin, cos or tan (n a nonzero multiple of 16 and |t| below 2^-27), where
 * t's 2^-91 is not enough relative accuracy; one argument in about 2^24 of the range
 */
static inline bool __libc_fast_trig_near_zero(uint32_t n, double t)
{
	return (n & 15) == 0 && n != 0 && isless(__builtin_fabs(t), 0x1p-27);
}

/* sin t - t = t sp and cos t - 1 = cp for |t| <= pi/64, from z = t^2: four terms each, fitted here (2^-65.3 relative to t, 2^-71.2) */
static inline double __libc_fast_sin_poly(double z)
{
	return z * (-0x1.5555555555555p-3 + z * (0x1.11111111110dfp-7 + z * (-0x1.a01a0199b464ep-13 + z * 0x1.71da14683ddd6p-19)));
}

static inline double __libc_fast_cos_poly(double z)
{
	return z * (-0x1.0000000000000p-1 + z * (0x1.55555555554cap-5 + z * (-0x1.6c16c1634b0cfp-10 + z * 0x1.a0144d7b73dccp-16)));
}

/*
 * A cos t + B sin t as hi + *lo, for (A, B) = (sin c, cos c) or (cos c, -sin c), c = j pi/32, j = 0..15: A + B_hi t
 * exactly (the product's error exact, then a fast two-sum: |A| >= 0.098 >= |B t| for j >= 1, and A or B is 0 for
 * j = 0), every other term in the low part; sp and cp as __libc_fast_sin_poly and __libc_fast_cos_poly give them
 */
static inline double __libc_fast_trig_kernel(double a_hi, double a_lo, double b_hi, double b_lo, double t, double t_lo, double sp, double cp, double *lo)
{
	struct __libc_dd product = __libc_dd_product(b_hi, t);
	struct __libc_dd sum = __libc_dd_fast_sum(a_hi, product.hi);

	*lo = sum.lo + product.lo + a_lo + b_lo * t + a_hi * cp + b_hi * (t * sp) + t_lo * (b_hi - a_hi * t);
	return sum.hi;
}

/*
 * +-sin(N pi/32 + t) as hi + *lo, without the sign of bit 5 of N (the caller's, with x's): bit 4 picks sin or cos of
 * a = j pi/32 + t, j = N mod 16
 */
static inline double __libc_fast_sin_turn(uint32_t n, double t, double t_lo, double sp, double cp, double *lo)
{
	uint32_t j = n & 15;
	const struct __libc_dd *s = &__libc_fast_sin_table[j];
	const struct __libc_dd *c = &__libc_fast_sin_table[16 - j];

	if ((n & 16) == 0)
		return __libc_fast_trig_kernel(s->hi, s->lo, c->hi, c->lo, t, t_lo, sp, cp, lo);

	return __libc_fast_trig_kernel(c->hi, c->lo, -s->hi, -s->lo, t, t_lo, sp, cp, lo);
}

/* hi + lo, negated first when negative: the sign on both parts before the one rounding */
static inline double __libc_fast_signed_sum(double hi, double lo, bool negative)
{
	return negative ? -hi - lo : hi + lo;
}

/* atan(j/16) for j = 0..16 as exact pairs (libc/math/fast/__libc_fast_atan_pairs.c, D198) */
extern const struct __libc_dd __libc_fast_atan_pairs[17];

/*
 * The double atan2 of the double-FPU cores' slow path (D198): the strict file's special cases, scaling and double-double
 * core, for what the fast path leaves (libc/math/fast/__libc_fast_atan2_slow.c)
 */
double __libc_fast_atan2_slow(double y, double x);

/* n / d + *lo: the quotient and its remainder over d, the remainder exact (the product's error by fma or Dekker) */
static inline double __libc_fast_quotient(double n, double d, double *lo)
{
	double q = n / d;
	struct __libc_dd p = __libc_dd_product(q, d);

	*lo = ((n - p.hi) - p.lo) / d;
	return q;
}

/*
 * atan(a + a_lo) as hi + *lo for 0 <= a <= 1 and |a_lo| a few units of a's last place (D198; the note, section 16):
 * c = j/16 for j = round(16 a) and atan(c) from the table; t = (a - c)/(1 + a c) with a - c exact, 1 + a c as a pair and
 * the quotient's remainder exact, |t| <= 1/32; atan t = t + t z P(z), z = t^2, five terms fitted here (2^-65.6 relative to
 * t); atan(c) + t by a fast two-sum (atan(1/16) > 1/32), the rest in the low part
 */
static inline double __libc_fast_atan_kernel(double a, double a_lo, double *lo)
{
	int j = (int)(a * 16.0 + 0.5);
	double c = (double)j * 0.0625;
	const struct __libc_dd *e = &__libc_fast_atan_pairs[j];
	struct __libc_dd p = __libc_dd_product(a, c);
	double d = 1.0 + p.hi;
	double d_lo = ((1.0 - d) + p.hi) + p.lo + a_lo * c;
	double u = a - c;
	double q = u / d;
	struct __libc_dd r = __libc_dd_product(q, d);
	double t_lo = (((u - r.hi) - r.lo) + a_lo - q * d_lo) / d;
	double z = q * q;
	double poly = z * (-0x1.5555555555555p-2 + z * (0x1.99999999998a4p-3 + z * (-0x1.24924922a7020p-3 + z * (0x1.c71c46c7ecbacp-4 + z * -0x1.739871480f801p-4))));
	struct __libc_dd sum = __libc_dd_fast_sum(e->hi, q);

	*lo = sum.lo + e->lo + t_lo + q * poly;
	return sum.hi;
}

/*
 * asin s - s = s w for z = s^2 <= 1/4 (D198), w = z/6 + z^2 Q(z) with Q of thirteen terms fitted here (2^-60.3 relative
 * to asin s): s w as hi + *lo with its leading part exact, z/6 by an exact product with 1/6 as a pair and s w_hi by
 * another, so that only z^2 Q (below 1 % of asin s) rounds in plain double. z + z_lo is s^2, as a pair.
 */
static inline double __libc_fast_asin_series(double s, double z, double z_lo, double *lo)
{
	double q = 0x1.023b2e4399215p-6 + z * (-0x1.c5cebe8c6ea21p-7 + z * 0x1.adba2f1957d13p-6);
	struct __libc_dd w;
	struct __libc_dd m;
	double w_lo;

	q = 0x1.2d1cc6b396421p-8 + z * q;
	q = 0x1.240b1269e5ee4p-7 + z * q;
	q = 0x1.3da52ba8cd821p-7 + z * q;
	q = 0x1.7ab9ababb452cp-7 + z * q;
	q = 0x1.c996a8c59b36cp-7 + z * q;
	q = 0x1.1c4ed306f4679p-6 + z * q;
	q = 0x1.6e8ba2987e0fap-6 + z * q;
	q = 0x1.f1c71c72b3a23p-6 + z * q;
	q = 0x1.6db6db6db652cp-5 + z * q;
	q = 0x1.3333333333334p-4 + z * q;
	w = __libc_dd_product(z, 0x1.5555555555555p-3);
	w_lo = w.lo + (z * 0x1.5555555555555p-57 + z_lo * 0x1.5555555555555p-3) + z * (z * q);
	m = __libc_dd_product(s, w.hi);
	*lo = m.lo + s * w_lo;
	return m.hi;
}

/*
 * The half-angle forms' asin s (D198): z = (1 - a)/2 exactly for 1/2 < a < 1, s = sqrt z correctly rounded, and asin s =
 * s + *rest + *tail_hi + *tail_lo: the square root's remainder (z - s^2)/(2 s) with s^2 exact, times 1 + w, and s w from
 * __libc_fast_asin_series
 */
static inline double __libc_fast_asin_half(double a, double *rest, double *tail_lo, double *tail_hi)
{
	double z = (1.0 - a) * 0.5;
	double s = __builtin_sqrt(z);
	struct __libc_dd square = __libc_dd_product(s, s);

	/*
	 * asin S = S (1 + w(z)) for S = sqrt z exactly, and the series takes s w(z): the remainder S - s carries the factor
	 * 1 + w = 1 + z/6 + 3 z^2/40 + ..., 1.045 at z = 1/4 (1 alone cost 2^-57.6 of the result next to 1/2)
	 */
	*rest = ((z - square.hi) - square.lo) / (s + s) * (1.0 + z * (0x1.5555555555555p-3 + z * 0x1.3333333333333p-4));
	*tail_hi = __libc_fast_asin_series(s, z, 0.0, tail_lo);
	return s;
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

/* 2^(j/32) as float pairs, and R(x) = erfc(x) e^(x^2) as degree-9 pieces (libc/math/sp-nearest/fast/) */
extern const float __libc_fast_exp2f_table[32][2];
extern const float __libc_fast_erfcf_r_table[7][15];

/* r + p as a pair, the sum's error and p's product error e added to *r_lo: one term of the accurate R below */
static inline float __libc_fast_add_term(float r, float *r_lo, float p, float e)
{
	float error;
	float sum = __libc_fast_two_sum(r, p, &error);

	*r_lo += error + e;
	return sum;
}

/*
 * erfc a = 2^k (return + *lo) for 0 <= a < 9.2 in float, to about 2^-32 relative, for a rounding test (the
 * single-precision FPU cores' erfcf; the trigonometric note, section 15). e^(-a^2): a^2 as an exact pair, n = round(-a^2
 * 32/ln2), u = -a^2 - n ln2/32 with the high product exact (ln2/32's float has its last bit at 2^-25, |u| <= ln2/64), and
 * e^u = 1 + u + u^2/2 + u^3/6 + u^4/24 (the next term 2^-39.6) times 2^(j/32) from the table as pairs, 2^((n - j)/32) =
 * 2^k. R from the table: c0 + c1 t + ... + c4 t^4 with each coefficient a pair, each term's product error by a fused
 * multiply-add and each sum by a two-sum (c4 t^4 reaches 2^-9.7 of R, and rounding it with the rest measured 2^-30.7),
 * then t^5 Q(t) in float (2^-12.5 of R); above 3, R =
 * v G(v^2) with v = 1/a and its remainder. The two pairs multiplied with the leading product's error by a fused
 * multiply-add and every cross term.
 */
static inline float __libc_fast_erfcf_accurate(float a, int *k, float *lo)
{
	float z = a * a;
	float z_lo = __builtin_fmaf(a, a, -z);
	float nf = __builtin_fmaf(-z, 0x1.715476p+5f, 0x1.8p23f) - 0x1.8p23f;
	float u = __builtin_fmaf(-nf, 0x1.62e430p-6f, -z);
	float u_lo = __builtin_fmaf(-nf, -0x1.05c610p-34f, -z_lo);
	int n = (int)nf;
	const float *table = __libc_fast_exp2f_table[n & 31];
	const float *c;
	float error;
	float p = __libc_fast_two_sum(1.0f, u, &error);
	/* The powers from u + u_lo: u_lo (a^2's rounding and n ln2/32's low part) reaches 4e-5 of u, and u_hi^2 alone cost 2^-29.6 */
	float w_u = u + u_lo;
	float tail = error + __builtin_fmaf(w_u * w_u, __builtin_fmaf(w_u, __builtin_fmaf(w_u, 0x1.555556p-5f, 0x1.555556p-3f), 0.5f), u_lo);
	float e = table[0] * p;
	float e_lo = __builtin_fmaf(table[0], p, -e) + __builtin_fmaf(table[0], tail, table[1] * p);
	float t;
	float t_lo;
	float tt;
	float tt_lo;
	float t3;
	float t3_lo;
	float t4;
	float t4_lo;
	float q;
	float r;
	float r_lo;
	float v = 1.0f;
	float v_lo = 0.0f;
	float w;
	float m;
	int i;

	/* (n - j)/32 exactly, n being negative or zero */
	*k = (n - (n & 31)) / 32;
	/*
	 * t = a - c, or w - c with w = 1/a^2, as a pair by a two-sum: a - 1/4 is inexact below 1/8 (no Sterbenz), and w is a
	 * pair itself (v^2 + 2 v v_lo); rounding t cost 2^-27 of R (measured); t_lo enters through R'(t) below
	 */
	if (a < 3.0f) {
		i = (int)(2.0f * a);
		t = __libc_fast_two_sum(a, -(0.25f + 0.5f * (float)i), &t_lo);
	} else {
		i = 6;
		v = 1.0f / a;
		v_lo = __builtin_fmaf(-v, a, 1.0f) / a;
		w = v * v;
		t = __libc_fast_two_sum(w, -0x1.f7811ap-5f, &t_lo);
		t_lo += __builtin_fmaf(v, v, -w) + 2.0f * v * v_lo;
	}
	c = __libc_fast_erfcf_r_table[i];
	q = __builtin_fmaf(c[14], t, c[13]);
	q = __builtin_fmaf(q, t, c[12]);
	q = __builtin_fmaf(q, t, c[11]);
	q = __builtin_fmaf(q, t, c[10]);
	tt = t * t;
	tt_lo = __builtin_fmaf(t, t, -tt);
	t3 = tt * t;
	t3_lo = __builtin_fmaf(tt, t, -t3) + tt_lo * t;
	t4 = tt * tt;
	t4_lo = __builtin_fmaf(tt, tt, -t4) + 2.0f * tt * tt_lo;
	r_lo = c[1] + __builtin_fmaf(t4 * t, q, t_lo * __builtin_fmaf(t, __builtin_fmaf(t, 3.0f * c[6], 2.0f * c[4]), c[2]));
	m = c[2] * t;
	r = __libc_fast_add_term(c[0], &r_lo, m, __builtin_fmaf(c[2], t, -m) + c[3] * t);
	m = c[4] * tt;
	r = __libc_fast_add_term(r, &r_lo, m, __builtin_fmaf(c[4], tt, -m) + __builtin_fmaf(c[4], tt_lo, c[5] * tt));
	m = c[6] * t3;
	r = __libc_fast_add_term(r, &r_lo, m, __builtin_fmaf(c[6], t3, -m) + __builtin_fmaf(c[6], t3_lo, c[7] * t3));
	m = c[8] * t4;
	r = __libc_fast_add_term(r, &r_lo, m, __builtin_fmaf(c[8], t4, -m) + __builtin_fmaf(c[8], t4_lo, c[9] * t4));
	if (i == 6) {
		/* R = v G(w), with v's remainder */
		m = v * r;
		r_lo = __builtin_fmaf(v, r, -m) + __builtin_fmaf(v, r_lo, v_lo * r);
		r = m;
	}
	/* Both pairs are unnormalised (e_lo carries e^u's u^2/2, r_lo R's t^4 Q, 2^-13 and 2^-11 of them): e_lo r_lo counts too */
	m = e * r;
	*lo = __builtin_fmaf(e, r, -m) + __builtin_fmaf(e, r_lo, __builtin_fmaf(e_lo, r, e_lo * r_lo));
	return m;
}

/* 2^(j/64) correctly rounded and (2^(j/64) - T)/T, as bits, side by side (libc/math/fast/__libc_fast_exp_pairs.c) */
extern const uint64_t __libc_fast_exp_pairs[128];

/*
 * e^u - 1 for |u| <= ln 2/128 and a little beyond (the reductions' error): u + u^2 q(u) with q of degree 3 fitted here
 * (Chebyshev, 256-bit arithmetic), u^2 q within 2^-57.7 of e^u - 1 - u (docs/research/libm-fast-double.md, section 6)
 */
static inline double __libc_fast_expm1_poly(double u)
{
	double u2 = u * u;

	return u + u2 * (0x1.ffffffffff57fp-2 + u * 0x1.5555555555255p-3 + u2 * (0x1.55556b3304ec0p-5 + u * 0x1.11111d8fbe766p-7));
}

/* 2^r - 1 for |r| <= 1/128 and a little beyond: r ln 2 + r^2 q(r), q of degree 3 fitted here (r^2 q within 2^-57.7) */
static inline double __libc_fast_exp2m1_poly(double r)
{
	double r2 = r * r;

	return r * 0x1.62e42fefa39efp-1 + r2 * (0x1.ebfbdff82bb76p-3 + r * 0x1.c6b08d7049cc0p-5 + r2 * (0x1.3b2acb2c36b23p-7 + r * 0x1.5d880e7706b57p-10));
}

/*
 * tail + (e^u - 1) and tail + (2^r - 1) by Horner's rule with the same fits, the tail added by the last multiply-add: on
 * the common paths this spares the square and the separate addition of the tail (two instructions per call)
 */
static inline double __libc_fast_expm1_tail(double u, double tail)
{
	return tail + u * (1.0 + u * (0x1.ffffffffff57fp-2 + u * (0x1.5555555555255p-3 + u * (0x1.55556b3304ec0p-5 + u * 0x1.11111d8fbe766p-7))));
}

static inline double __libc_fast_exp2m1_tail(double r, double tail)
{
	return tail + r * (0x1.62e42fefa39efp-1 + r * (0x1.ebfbdff82bb76p-3 + r * (0x1.c6b08d7049cc0p-5 + r * (0x1.3b2acb2c36b23p-7 + r * 0x1.5d880e7706b57p-10))));
}

/* The table entry of 2^(k/64): the bits of 2^(j/64) and of its tail, j = k mod 64 */
static inline const uint64_t *__libc_fast_exp_entry(int32_t k)
{
	return &__libc_fast_exp_pairs[2 * (k & 63)];
}

/* k = round(64 x / c) for |x / c| below 2^30 by the 1.5 2^52 shift (t = 64 x / c), and k itself as a double in *kd */
static inline int32_t __libc_fast_exp_index(double t, double *kd)
{
	double shifted = t + 0x1.8p52;

	*kd = shifted - 0x1.8p52;
	return (int32_t)(uint32_t)__libc_double_bits(shifted);
}

/* The bits of 2^(k/64) from its table entry and exponent n = floor(k/64) */
static inline uint64_t __libc_fast_exp_scale_bits(const uint64_t *entry, int32_t n)
{
	return entry[0] + ((uint64_t)(int64_t)n << 52);
}

/*
 * 2^(k/64) (1 + p) rounded once for |p| < 2^-6, as scale + scale (tail + p), the ends tested (the common paths, below
 * 2^1014 in magnitude, build the scale themselves from __libc_fast_exp_entry with no test): the tail is below 2^-53 and its product with p below 2^-59, so before the last
 * addition the error is about 2^-57 of the result. From 2^1023 the scale is halved and the sum doubled (exact, or the
 * overflow); below 2^-1014 it is scaled by 2^1022 and the sum multiplied by 2^-1022, a second rounding only to the
 * subnormal grid (and the underflow when it is inexact): unscaled, scale tmp would already be subnormal there and lose
 * bits (0.76 ulp measured at 2^-1020).
 */

/*
 * The top 11 bits of |x| (its biased exponent) outside [bottom, top): the fast functions' one test for their slow paths
 * (tiny arguments, the ends, the infinities and NaN)
 */
static inline bool __libc_fast_exp_outside(double x, uint32_t bottom, uint32_t top)
{
	return (((uint32_t)(__libc_double_bits(x) >> 52) & 0x7ff) - bottom) >= top - bottom;
}

static inline double __libc_fast_exp_scale(int32_t k, double p)
{
	const uint64_t *entry = &__libc_fast_exp_pairs[2 * (k & 63)];
	int32_t n = k >> 6;
	double tmp = __libc_double_from(entry[1]) + p;
	double scale;

	if (n > 1022) {
		scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, n - 1));
		return 2.0 * (scale + scale * tmp);
	}
	if (n < -1014) {
		scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, n + 1022));
		return (scale + scale * tmp) * 0x1p-1022;
	}
	scale = __libc_double_from(__libc_fast_exp_scale_bits(entry, n));
	return scale + scale * tmp;
}

/*
 * The slow paths of the fast exp, exp2 and exp10 (libc/math/fast/ and libc/math/dp-fixed/fast/, PLAN D187, D188): tiny
 * arguments, the ends, the infinities and NaN, by the strict file's double-double core (rare, and the core is shared with
 * pow, so in a program that calls both it costs no code). Out of line, in their own files, so that the common path saves
 * nothing and the float-only layers, which include this header, carry no double code (their check refuses it at -O0)
 */
double __libc_fast_exp_special(double x);
double __libc_fast_exp2_special(double x);
double __libc_fast_exp10_special(double x);

/*
 * The fast double logarithms (libc/math/fast/, PLAN D190): x = 2^k z with z in [0x1.6955p-1, 0x1.6955p+0), the
 * entry by the top 7 bits of z's fraction, r = z invc - 1 exactly (invc of at most 10 bits, chosen per entry so that
 * |r| <= 2^-7.47 and r fits 53 bits), and -log(invc) in the table's units as logc, a multiple of 2^-43 (2^-42 for log2)
 * so that k ln 2 + logc is exact, plus its tail
 */
struct __libc_fast_log_entry {
	double invc;
	double logc;
	double tail;
};

/* In natural logarithms (__libc_fast_log_table.c) and in base 2 (__libc_fast_log2_table.c) */
extern const struct __libc_fast_log_entry __libc_fast_log_table[128];
extern const struct __libc_fast_log_entry __libc_fast_log2_table[128];

/* The logarithms of x <= 0, +inf and NaN (C23 7.12.6.11, F.10.3.7): -inf with divide-by-zero, invalid, inf, NaN */
double __libc_fast_log_special(double x);

/*
 * The entry for a positive normal x's bits (or a subnormal's, scaled by 2^52 and the exponent taken back): z and k with
 * x = 2^k z, the offset putting 1 two thirds of the way into its entry, whose invc is 1
 */
static inline const struct __libc_fast_log_entry *__libc_fast_log_reduce(const struct __libc_fast_log_entry *table, uint64_t ix, double *z, int32_t *k)
{
	uint64_t tmp = ix - 0x3fe68aaa00000000ull;

	*k = (int32_t)((int64_t)tmp >> 52);
	*z = __libc_double_from(ix - (tmp & 0xfff0000000000000ull));

	return &table[(tmp >> 45) & 127];
}

/*
 * r = z invc - 1, exact: one fused multiply-add, or without one z split after its 43rd bit, so that both products are
 * exact (invc has at most 10 bits), the first minus 1 is exact (Sterbenz) and so is the sum, r being representable
 */
static inline double __libc_fast_log_r(double z, double invc)
{
	double high;

	if (__libc_fpu_fma)
		return __builtin_fma(z, invc, -1.0);

	high = __libc_double_from(__libc_double_bits(z) & ~0x3ffull);
	return (high * invc - 1.0) + (z - high) * invc;
}

/*
 * (log1p(r) - r)/r^2 for |r| <= 2^-7.47 by its Chebyshev fit of degree 5 in double (r^2 times it within 2^-60.1 of
 * log1p(r), relatively), in Estrin's order: log, log10 and log1p
 */
static inline double __libc_fast_log_poly(double r, double r2)
{
	return -0x1.0000000000001p-1 + r * 0x1.5555555555557p-2 + r2 * (-0x1.fffffffd8e049p-3 + r * 0x1.999999976d2bcp-3 + r2 * (-0x1.555875f84f672p-3 + r * 0x1.249510d200b2dp-3));
}

/*
 * True for x <= 0, +inf and NaN (__libc_fast_log_special); otherwise a positive normal x's bits in *ix, or a subnormal's
 * scaled by 2^52 with the exponent taken back (which can make any bit pattern, 0 included for 2^-1023)
 */
static inline bool __libc_fast_log_outside(double x, uint64_t *ix)
{
	*ix = __libc_double_bits(x);
	if (*ix - 0x0010000000000000ull >= 0x7fe0000000000000ull) {
		if (*ix == 0 || *ix >= 0x7ff0000000000000ull)
			return true;
		*ix = __libc_double_bits(x * 0x1p52) - (52ull << 52);
	}

	return false;
}

/* 0 for a non-integer y, 1 for an odd integer, 2 for an even one (|y| finite) */
static inline int __libc_fast_pow_integer_kind(uint64_t iy)
{
	int32_t e = (int32_t)((iy >> 52) & 0x7ff) - 1023;

	if (e < 0)
		return (iy << 1) == 0 ? 2 : 0;
	if (e > 52)
		return 2;
	if ((iy & ((1ull << (52 - e)) - 1)) != 0)
		return 0;

	return ((iy >> (52 - e)) & 1) != 0 && e < 53 ? 1 : 2;
}

/*
 * The fast pows' special cases that need no logarithm (__libc_fast_pow_special.c; libc/math/fast/pow.c and
 * libc/math/dp-fixed/fast/pow.c): y = +-0, x = 1, NaN, an infinite y, x = +-0 or +-inf, x < 0 with y not an integer,
 * and |y| below 2^-65 or from 2^63 for a finite nonzero x
 */
double __libc_fast_pow_special(double x, double y);

#endif /* _MATH_FAST_H */
