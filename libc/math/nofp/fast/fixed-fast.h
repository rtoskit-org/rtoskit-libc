/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fixed-fast.h
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FIXED_FAST_H
#define _FIXED_FAST_H

#include <stdbit.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"

/*
 * The fast flavour's fixed-point helpers of the cores without an FPU (PLAN D173, D174; docs/research/libm-fast-float-exp-log.md
 * sections 16 and 17): a float from a 64-bit significand, a series near 0 for the m1 and p1 forms, 2^t - 1 from the exp2
 * core and 1 + x exactly for the log2 core
 */

/*
 * The float nearest m 2^(e - 63), m with bit 63 set, for a normal result: the top 24 bits rounded by the next one (a carry
 * reaches the exponent through the addition)
 */
static inline float __libc_fixed_float(uint64_t m, int e, bool negative)
{
	uint32_t significand = (uint32_t)(m >> 40) + (uint32_t)((m >> 39) & 1);

	return __libc_float_from((((uint32_t)(e + 126) << 23) + significand) | (negative ? 0x80000000u : 0));
}

/*
 * The float nearest m 2^(e - 63), m with bit 63 set, for e < -126: the bits down to 2^-149 rounded by the next one, once
 * (a subnormal or zero result; a carry gives FLT_MIN). Kept apart from __libc_fixed_float and off the series' path: one
 * helper for both was not inlined any more, 3 to 5 % slower on the m1 forms, and the check in the series 1 to 2 % (measured)
 */
static inline float __libc_fixed_float_subnormal(uint64_t m, int e, bool negative)
{
	int shift = 40 - 126 - e;
	uint32_t significand;

	if (shift < 64)
		significand = (uint32_t)(m >> shift) + (uint32_t)((m >> (shift - 1)) & 1);
	else
		significand = shift == 64 ? (uint32_t)(m >> 63) : 0;
	return __libc_float_from(significand | (negative ? 0x80000000u : 0));
}

/*
 * The float nearest m 2^e for a nonzero m, normal or subnormal (the tiny arguments' x c, c a 64-bit constant: below 2^-60
 * e^u - 1 is u and log(1 + x) is x to far below an ulp)
 */
static inline float __libc_fixed_product_float(uint64_t m, int e, bool negative)
{
	int k = (int)stdc_leading_zeros(m);

	if (e + 63 - k < -126)
		return __libc_fixed_float_subnormal(m << k, e + 63 - k, negative);
	return __libc_fixed_float(m << k, e + 63 - k, negative);
}

/* The significand of a nonzero finite |x| normalised to bit 31, and *e with |x| = m 2^(*e - 31), subnormals included */
static inline uint32_t __libc_fixed_significand(uint32_t magnitude, int *e)
{
	int exponent = (int)(magnitude >> 23);
	uint32_t m = magnitude & __LIBC_FLOAT_FRACTION;

	if (exponent == 0) {
		int k = (int)stdc_leading_zeros(m) - 8;

		m <<= k;
		exponent = 1 - k;
	} else {
		m |= 0x00800000;
	}
	*e = exponent - 127;
	return m << 8;
}

/*
 * x c rounded once for a nonzero tiny x, c = (high 2^32 + low) 2^(offset - 32) a 64-bit constant (low may be 0), subnormal
 * results included: out of line, so that the rare tiny path leaves the callers' common paths as they were (inline, it cost
 * them 1 to 2 % on the m1 forms, measured)
 */
[[gnu::noinline, maybe_unused]] static float __libc_fixed_tiny(uint32_t magnitude, bool negative, uint32_t high, uint32_t low, int offset)
{
	int e;
	uint32_t m = __libc_fixed_significand(magnitude, &e);

	return __libc_fixed_product_float(__libc_fixed_wide(m, high) + (__libc_fixed_wide(m, low) >> 32), e + offset, negative);
}

/*
 * u K(u) for 2^-62 <= |u| < 2^-3 (a normal result), u = mu 2^(eu - 31) with bit 31 of mu set, K = leading + u p(u) with p(u) the polynomial of
 * the six coefficients (p(u) = c0 + c1 u + ... + c5 u^5) in signed Q31 by Horner's rule with u in Q34 and leading in
 * unsigned Q31; K in (0, 2): mu K as a 64-bit product, normalised by its leading zeros and rounded once. The relative error
 * stays about 2^-30 however small u is.
 */
static inline float __libc_fixed_series(uint32_t mu, int eu, bool negative, const int32_t coefficients[6], uint32_t leading)
{
	int shift = -3 - eu;
	int32_t u = shift < 32 ? (int32_t)(mu >> shift) : 0;
	int32_t p = coefficients[5];
	uint64_t m;
	int k;

	if (negative)
		u = -u;
	for (int i = 4; i >= 0; --i)
		p = coefficients[i] + (__libc_fixed_high_signed(u, p) >> 2);
	m = __libc_fixed_wide(mu, leading + (uint32_t)(__libc_fixed_high_signed(u, p) >> 2));
	k = (int)stdc_leading_zeros(m);

	return __libc_fixed_float(m << k, eu + 1 - k, negative);
}

/*
 * e^u - 1 = u (1 + u p(u)) for 2^-62 <= |u| < 2^-3, p = (e^u - 1 - u)/u^2 by its Taylor series to u^5 (the next term below
 * 2^-33 relative): 1 + u p in (0.93, 1.07)
 */
static inline float __libc_expm1f_series_fixed(uint32_t mu, int eu, bool negative)
{
	static const int32_t coefficients[6] = { 0x40000000, 0x15555555, 0x05555555, 0x01111111, 0x002d82d8, 0x00068068 };

	return __libc_fixed_series(mu, eu, negative, coefficients, 0x80000000u);
}

/*
 * 2^t - 1 for z = t 2^37 with |t| >= 0.18 and t >= -30: the exp2 core gives 2^n (1 + f 2^-32), within 6 units of f; for
 * |n| <= 30, (2^n - 1) + 2^n f 2^-32 is exact in 64 bits (units of 2^-32, or of 2^(n - 32) below 1), normalised and
 * rounded once, its relative error at most 6 2^(n - 32) / |2^t - 1|, below 2^-28.4 for |t| >= 0.18; beyond 2^30 the 1
 * is below 2^-30 of the result
 */
static inline float __libc_exp2m1f_fixed(int64_t z)
{
	int n;
	uint32_t f = __libc_exp2f_fixed(z, &n);
	uint64_t one = UINT64_C(1) << 32;
	int64_t v;
	uint64_t m;
	int e;
	int k;

	if (n > 30)
		return __libc_float_from(__libc_fixed_nearest(f, n));
	if (n >= 0) {
		v = (int64_t)(((f + one) << n) - one);
		e = -32;
	} else {
		v = (int64_t)(f + one) - (int64_t)(UINT64_C(1) << (32 - n));
		e = n - 32;
	}
	m = v < 0 ? -(uint64_t)v : (uint64_t)v;
	k = (int)stdc_leading_zeros(m);

	return __libc_fixed_float(m << k, e + 63 - k, v < 0);
}

/*
 * 1 + x for a finite x > -1 with |x| >= 2^-5 as 2^*e m, m in [sqrt(1/2), sqrt(2)) as an unsigned Q31 number for
 * __libc_log2_fixed_core: up to 2^23, 1 + x is exact in units of x's ulp (at least 2^-28, so below 2^29), with up to 30
 * significant bits; beyond, the 1 is below x's ulp, and log(1 + x) - log x < 1/x is below 2^-28 of log x: x itself
 */
static inline uint32_t __libc_fixed_one_plus(uint32_t bits, int *e)
{
	int exponent = (int)((bits & 0x7fffffff) >> 23) - 127;
	uint32_t significand = (bits & __LIBC_FLOAT_FRACTION) | 0x00800000;
	uint32_t m;

	if (exponent <= 23) {
		uint32_t one = UINT32_C(1) << (23 - exponent);
		uint32_t sum = (bits >> 31) != 0 ? one - significand : one + significand;
		int k = (int)stdc_leading_zeros(sum);

		m = sum << k;
		*e = exponent + 8 - k;
	} else {
		m = significand << 8;
		*e = exponent;
	}
	/* Halved above sqrt(2): its low bits are zeros (at most 30 significant bits), so the shift is exact */
	if (m > 0xb504f333u) {
		m >>= 1;
		++*e;
	}
	return m;
}

#endif /* _FIXED_FAST_H */
