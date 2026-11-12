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

/*
 * The fast trigonometric functions of the cores without an FPU (PLAN D180; docs/research/libm-fast-float-trig.md section
 * 16): a fixed-point reduction to k pi/32 + s with s in units of 2^-48, nofp/speed's sine kernel (PLAN D87) on that s, and
 * the result rounded to nearest. pi/32 in units of 2^-67, rounded, computed here
 */
#define __LIBC_FIXED_PI_32 0xc90fdaa22168c235ULL

/* Bits [position, position + 32) of a little-endian array of words, position >= 0 */
static inline uint32_t __libc_fixed_word_at(const uint32_t *words, int position)
{
	int index = position / 32;
	int shift = position % 32;

	if (shift == 0)
		return words[index];
	return (words[index] >> shift) | (words[index + 1] << (32 - shift));
}

/* The high 64 bits of a 32 x 64-bit product, the low word's carry included */
static inline uint64_t __libc_fixed_wide_high(uint32_t a, uint64_t b, uint32_t *low)
{
	uint64_t lo = __libc_fixed_wide(a, (uint32_t)b);
	uint64_t hi = __libc_fixed_wide(a, (uint32_t)(b >> 32)) + (lo >> 32);

	*low = (uint32_t)lo;
	return hi;
}

/*
 * |x| = (k + f) pi/32 for 2^-6 <= |x| < 2^20, |f| <= 1/2: x = m 2^(e - 23) with m the 24-bit significand, and x 32/pi =
 * m T 2^(e - 147) with T = floor(32/pi 2^124), a 152-bit product whose integer part starts at bit 147 - e; T's truncation
 * adds below 2^-100 to the fraction. The nearest integer k (the kernel takes it modulo 64) and the signed fraction to 2^-64,
 * times pi/32 to 64 bits: s in units of 2^-48, its error below one unit; the sign of x on both (out of line in
 * __libc_fixed_trig_reduce.c, which sinf and cosf share in one program)
 */
int __libc_fixed_trig_reduce(uint32_t bits, int64_t *s);

/*
 * cos s - 1 and sin s/s - 1 for |s| <= pi/64 + 2^-40 in units of 2^-48, as nofp/speed's __libc_sinf_fixed forms them (its
 * comment, D87): u = s^2 in units of 2^-38, cos s - 1 = u (-1/2 + u/24 - u^2/720) and sin s/s - 1 = u (-1/6 + u/120) in
 * units of 2^-37. They do not depend on k, so sin and cos of one s share them
 */
static inline void __libc_fixed_trig_terms(int64_t s, int32_t *cosine_m1, int32_t *g)
{
	int32_t s35 = (int32_t)(s >> 13);
	int32_t u = __libc_fixed_high_signed(s35, s35);
	int32_t h = 89478485 - (int32_t)(__libc_fixed_high((uint32_t)u, 2982616) >> 6);

	h = -(1 << 30) + (__libc_fixed_high_signed(u, h) >> 6);
	*cosine_m1 = __libc_fixed_high_signed(u, h);
	*g = -357913941 + (int32_t)(__libc_fixed_high((uint32_t)u, 17895697) >> 6);
	*g = __libc_fixed_high_signed(u, *g);
}

/*
 * sin(k pi/32 + s) for any k (modulo 64) from s and its terms, nofp/speed's __libc_sinf_fixed (D87) on s itself rather than
 * a float pair, the speed file keeping its own copy: k + 8 = 16 q + (j + 8) gives the quadrant q and the table angle
 * a = |j| pi/32, and the value first (1 + (cos s - 1)) + second sin s with first and second the signed sin a or cos a, in
 * units of 2^-62. The result v 2^(*exponent - 63) with bit 63 of v set; 0 when |s| < 2^-14 in a sine quadrant with a = 0,
 * where the result needs s to more bits (out of line in __libc_fixed_trig_combine.c: inlined in each caller, sinf and cosf
 * together exceeded math-mix's locked fast figure by 604 bytes on armv6-m)
 */
uint64_t __libc_fixed_trig_combine(int k, int64_t s, int32_t cosine_m1, int32_t g, int *exponent, bool *negative);

/* sin(k pi/32 + s): the terms and the combination */
static inline uint64_t __libc_fixed_trig_kernel(int k, int64_t s, int *exponent, bool *negative)
{
	int32_t cosine_m1;
	int32_t g;

	__libc_fixed_trig_terms(s, &cosine_m1, &g);
	return __libc_fixed_trig_combine(k, s, cosine_m1, g, exponent, negative);
}

/*
 * The pi forms' s = r 2^-shift pi/32 in units of 2^-48 for |r| 2^-shift <= 1/2, r below 2^24, shift >= 0: r times pi/32 to
 * 64 bits, the 96-bit product kept in high:low for __libc_fixed_pi_small
 */
static inline int64_t __libc_fixed_pi_s(int32_t r, int shift, uint64_t *high, uint32_t *low)
{
	uint32_t magnitude = r < 0 ? -(uint32_t)r : (uint32_t)r;
	int64_t s = 0;

	*high = __libc_fixed_wide_high(magnitude, __LIBC_FIXED_PI_32, low);
	/* r Pi 2^-67 2^-shift 2^48 = (high:low) >> (19 + shift) */
	if (shift + 19 < 32)
		s = (int64_t)((*high << (13 - shift)) | (*low >> (19 + shift)));
	else if (shift + 19 < 96)
		s = (int64_t)(*high >> (shift + 19 - 32));
	return r < 0 ? -s : s;
}

/*
 * Where the kernel declines (a sine quadrant with a = 0 and |s| < 2^-14): +-sin s = +-s (1 - s^2/6) with s from r times
 * pi/32 to 64 bits (high:low), relative however small; s^2/6 is below 2^-30.6 there and left out
 */
static inline uint64_t __libc_fixed_pi_small(int k, int32_t r, int shift, uint64_t high, uint32_t low, int *exponent, bool *negative)
{
	int zeros = (int)stdc_leading_zeros(high);
	uint64_t v = zeros == 0 ? high : (high << zeros) | ((uint64_t)low >> (32 - zeros));

	/* r Pi = v 2^(32 - zeros) (its leading one at bit 95 - zeros); the value r Pi 2^(-67 - shift) is v 2^(exponent - 63) */
	*exponent = 28 - zeros - shift;
	*negative = (r < 0) != ((k & 32) != 0);
	return v;
}

/* sin((k + r 2^-shift) pi/32) for the pi forms: the kernel, or __libc_fixed_pi_small where it declines */
static inline uint64_t __libc_fixed_sinpi(int k, int32_t r, int shift, int *exponent, bool *negative)
{
	uint64_t high;
	uint32_t low;
	int64_t s = __libc_fixed_pi_s(r, shift, &high, &low);
	uint64_t v = __libc_fixed_trig_kernel(k, s, exponent, negative);

	return v != 0 ? v : __libc_fixed_pi_small(k, r, shift, high, low, exponent, negative);
}

/*
 * 32x = K + r 2^-shift exactly for 2^-90 <= |x| < 2^23, K the nearest integer (ties up in magnitude) and |r| 2^-shift <= 1/2,
 * r below 2^24: x = m 2^(e - 23), 32x = m 2^(e - 18), from the bits; both signed as x. *r is 0 from 2^18 up
 */
static inline int __libc_fixed_pi_split(uint32_t bits, int32_t *r, int *shift)
{
	int32_t m = (int32_t)((bits & 0x007fffff) | 0x00800000);
	int e = (int)((bits >> 23) & 0xff) - 127;
	int32_t k;

	*shift = 18 - e;
	if (*shift <= 0) {
		k = m << -*shift;
		*r = 0;
		*shift = 0;
	} else if (*shift > 24) {
		k = 0;
		*r = m;
	} else {
		k = (m + (1 << (*shift - 1))) >> *shift;
		*r = m - (k << *shift);
	}
	if ((bits >> 31) != 0) {
		k = -k;
		*r = -*r;
	}
	return k;
}

/* The kernel's result rounded to nearest (a normal float): its 32 bits below the leading one, as __libc_fixed_nearest */
static inline float __libc_fixed_trig_float(uint64_t v, int exponent, bool negative)
{
	return __libc_float_from(__libc_fixed_nearest((uint32_t)(v >> 31), exponent) | (negative ? 0x80000000u : 0));
}

/*
 * sine / cosine from two kernel results rounded to nearest, as nofp/speed's tanf divides them (its comment, D87): the
 * quotient by the cosine's top 32 bits, one more bit from the remainder, less q times the cosine's low half
 */
static inline float __libc_fixed_trig_divide(uint64_t sine, int sine_exponent, bool sine_negative, uint64_t cosine, int cosine_exponent, bool cosine_negative)
{
	uint32_t top = (uint32_t)(cosine >> 32);
	uint32_t low = (uint32_t)cosine;
	uint64_t quotient = sine / top;
	uint64_t remainder = sine - quotient * top;
	uint64_t correction;
	int exponent = sine_exponent - cosine_exponent;

	if ((quotient >> 32) == 0) {
		quotient = (quotient << 1) | ((remainder << 1) >= top);
		--exponent;
	}
	correction = low + (uint64_t)__libc_fixed_high((uint32_t)quotient, low);
	while (correction >= top) {
		correction -= top;
		--quotient;
	}
	return __libc_float_from(__libc_fixed_nearest((uint32_t)quotient, exponent) | (sine_negative != cosine_negative ? 0x80000000u : 0));
}

/* pi and pi/2 in units of 2^-62, and 2^33/pi, rounded (computed here) */
#define __LIBC_FIXED_PI_62 0xc90fdaa22168c235ULL
#define __LIBC_FIXED_HALF_PI_62 0x6487ed5110b4611aULL
#define __LIBC_FIXED_INVERSE_PI_33 0xa2f9836eU

/*
 * n/d in Q32 for significands with bit 31 set and n < d (a quotient in [1/2, 1)): 1/d from the linear seed 24/17 - 8/17 d
 * (2^-4.1 relative for d in [1, 2)) and two Newton steps r + r (1 - d r) (2^-16), then the quotient n r corrected once
 * by its exact remainder, (n - q d) r, rounded (2^-31). No division instruction, which Armv6-M lacks (a 64-bit division by the
 * library call costs about 900 instructions there). d = 1 is exact.
 */
static inline uint32_t __libc_fixed_quotient(uint32_t n, uint32_t d)
{
	uint64_t remainder;
	uint32_t magnitude;
	uint32_t q;
	uint32_t r;
	int32_t e;

	if (d == 0x80000000u)
		return n << 1;

	r = (uint32_t)(UINT64_C(6063483241) - __libc_fixed_high(d, 4042322161u));
	for (int i = 0; i < 2; ++i) {
		/* 1 - d r in Q32, at most 1/17; the step never takes r above 1/d */
		e = (int32_t)((int64_t)((UINT64_C(1) << 63) - __libc_fixed_wide(d, r)) >> 31);
		r = e >= 0 ? r + __libc_fixed_high(r, (uint32_t)e) : r - __libc_fixed_high(r, -(uint32_t)e);
	}

	/* q = n r in Q32; n - q d in units of 2^-63 is below 2^48, and (n - q d) r its correction */
	q = (uint32_t)(__libc_fixed_wide(n, r) >> 31);
	remainder = ((uint64_t)n << 32) - __libc_fixed_wide(q, d);
	if ((int64_t)remainder >= 0) {
		magnitude = (uint32_t)(remainder >> 17);
		return q + ((__libc_fixed_high(magnitude, r) + (1 << 13)) >> 14);
	}
	magnitude = (uint32_t)(-remainder >> 17);
	q -= (__libc_fixed_high(magnitude, r) + (1 << 13)) >> 14;

	/* The quotient is at least 1/2: a unit below (n = d/2 exactly) would take the caller below its range */
	return q < 0x80000000u ? 0x80000000u : q;
}

/*
 * atan(a/b) for float magnitudes 0 < a <= b < infinity as their bits (subnormals included): v 2^(*exponent - 63) with
 * bit 63 of v set, relative error about 2^-31 however small a/b (out of line in __libc_fixed_atan.c, which atanf, atan2f
 * and their pi forms share)
 */
uint64_t __libc_fixed_atan(uint32_t a, uint32_t b, int *exponent);

/* theta/pi for theta = v 2^(*exponent - 63), bit 63 of v set: its top 32 bits times 2^33/pi, normalised */
static inline uint64_t __libc_fixed_inverse_pi(uint64_t v, int *exponent)
{
	uint64_t w = __libc_fixed_wide((uint32_t)(v >> 32), __LIBC_FIXED_INVERSE_PI_33);
	int k = (int)stdc_leading_zeros(w);

	*exponent -= 1 + k;
	return w << k;
}

/*
 * offset - theta or offset + theta rounded to nearest, theta = v 2^(exponent - 63) below 1 and offset (pi, pi/2, 1 or 1/2)
 * in units of 2^-62: the result is at least 1/4, so theta below 2^-62 counts for nothing
 */
static inline float __libc_fixed_offset_float(uint64_t offset, uint64_t v, int exponent, bool subtract, bool negative)
{
	int shift = 1 - exponent;
	uint64_t theta = shift < 64 ? v >> shift : 0;
	uint64_t r = subtract ? offset - theta : offset + theta;
	int k = (int)stdc_leading_zeros(r);

	return __libc_fixed_float(r << k, 1 - k, negative);
}

/*
 * The arcsine kernel for a float magnitude 0 < |x| <= 1 as its bits (out of line in __libc_fixed_asin.c, which asinf, acosf
 * and their pi forms share): up to 1/2 asin |x| (*outer false), above 2 asin sqrt((1 - |x|)/2) (*outer true, 0 for |x| = 1),
 * as v 2^(*exponent - 63) with bit 63 of v set, relative error about 2^-31
 */
uint64_t __libc_fixed_asin(uint32_t magnitude, int *exponent, bool *outer);

#endif /* _FIXED_FAST_H */
