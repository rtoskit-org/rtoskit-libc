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

#endif /* _MATH_FAST_H */
