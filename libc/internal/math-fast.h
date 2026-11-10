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
 * e^(u_hi + u_lo) * 2^k in float (the single-precision FPU cores, libc/math/sp-nearest/fast/) for
 * |u_hi + u_lo| <= ln2/2 (a little more is fine), u_lo much smaller than u_hi, -153 <= k <= 130. With
 * q(u) ~ (e^u - 1 - u)/u^2 of degree 5 (relative error 2^-30.3 on e^u), e^u = 1 + (u_hi + (u_lo + u^2 q(u))): u_hi
 * enters exactly and the rest is small, so the result is below 1 ulp before scaling (the note, section 4).
 */
static inline float __libc_fast_expf_kernel(float u_hi, float u_lo, int k)
{
	float u = u_hi + u_lo;
	float q = __builtin_fmaf(u, 0x1.a12528p-13f, 0x1.6d4362p-10f);
	float p;

	q = __builtin_fmaf(u, q, 0x1.1110ep-7f);
	q = __builtin_fmaf(u, q, 0x1.5554e8p-5f);
	q = __builtin_fmaf(u, q, 0x1.555556p-3f);
	q = __builtin_fmaf(u, q, 0x1p-1f);
	p = 1.0f + (u_hi + __builtin_fmaf(u * u, q, u_lo));

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
 * log(m) in double for m in [sqrt(1/2), sqrt(2)) (the double-FPU cores, libc/math/fast/): s = (m - 1)/(m + 1), exact
 * but for the division's rounding, and log m = 2 atanh s = 2s (1 + s^2 Q(s^2)) with the Taylor coefficients 1/3 to
 * 1/13: relative error 2^-39.5 (the note, section 7)
 */
static inline double __libc_fast_log_double(double m)
{
	double f = m - 1.0;
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

#endif /* _MATH_FAST_H */
