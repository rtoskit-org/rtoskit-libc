/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * dp-fixed-fast.h
 *
 * Created on: Nov 12, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _DP_FIXED_FAST_H
#define _DP_FIXED_FAST_H

#include <stdbool.h>
#include <stdint.h>

#include "internal/math.h"
#include "trig-fixed.h"

/*
 * The fast flavour's 64-bit fixed-point double kernels of the soft-double cores (PLAN D186, D188;
 * docs/research/libm-fast-double.md, section 7), over the speed files' 32 x 32 -> 64 product (__libc_trig_wide) and its
 * high-64 product (__libc_trig_high64)
 */

/* 2^(j/64) for j = 0..63 in Q63, rounded (libc/math/dp-fixed/fast/__libc_dp_fixed_exp2_table.c) */
extern const uint64_t __libc_dp_fixed_exp2_table[64];

/*
 * 2^(n + f 2^-64) rounded to nearest for a normal result, |n| <= 1021: 2^(j/64) from the table for the top 6 bits of f,
 * 2^r - 1 for the rest by its Taylor polynomial in Q64, one rounding to the double's bits (out of line in
 * __libc_dp_fixed_exp2.c, which exp, exp2 and exp10 share)
 */
double __libc_dp_fixed_exp2(int32_t n, uint64_t f);

/*
 * 2^(f 2^-64) in Q63, in [2^63, 2^64), its error a few units (the table's half unit, the products' floors): 2^(j/64) from
 * the table for the top 6 bits of f, times 1 + P with P = 2^r - 1 for r in [0, 1/64). P is the sum of (r ln 2)^k/k!: with
 * u = 64 r in Q64, the terms' coefficients (ln 2/64)^k/k! in Q64, k = 1..7 (the eighth is below 0.09 units of 2^-64). All
 * positive, so Horner's rule runs unsigned; the three highest fit in 32 bits, so their products take two partial products,
 * not four. The kernel's (__libc_dp_fixed_exp2) and expm1's, which subtracts 1 from it in integers.
 */
static inline uint64_t __libc_dp_fixed_exp2_fraction(uint64_t f)
{
	constexpr uint64_t a1 = 0x02c5c85fdf473de7ull;
	constexpr uint64_t a2 = 0x0003d7f7bff058b2ull;
	constexpr uint64_t a3 = 0x0000038d611ae094ull;
	constexpr uint64_t a4 = 0x0000000276556df7ull;
	constexpr uint64_t a5 = 0x00000000015d87feull;
	constexpr uint64_t a6 = 0x000000000000a185ull;
	constexpr uint64_t a7 = 0x0000000000000040ull;
	uint64_t t = __libc_dp_fixed_exp2_table[f >> 58];
	uint64_t u = f << 6;
	uint64_t p;

	p = a6 + __libc_trig_high64(u, a7);
	p = a5 + __libc_trig_high64(u, p);
	p = a4 + __libc_trig_high64(u, p);
	p = a3 + __libc_trig_high64(u, p);
	p = a2 + __libc_trig_high64(u, p);
	p = a1 + __libc_trig_high64(u, p);
	p = __libc_trig_high64(u, p);

	return t + __libc_trig_high64(t, p);
}

/*
 * |x| c as n + f 2^-64 for |x| = m 2^(e - 52) (m the 53-bit significand) and c = (c_hi + c_lo 2^-64) 2^-point, c_hi with
 * bit 63 set: m c_hi exactly in 117 bits plus the high part of m c_lo, which is |x| c 2^(52 - e + point), then shifted
 * by 52 - e + point - 64; below 2^-64 the bits are dropped (the error is below 2^-63 of the result)
 */
static inline uint64_t __libc_dp_fixed_scaled(uint64_t m, int e, uint64_t c_hi, uint64_t c_lo, int point, int32_t *n)
{
	uint32_t m0 = (uint32_t)m;
	uint32_t m1 = (uint32_t)(m >> 32);
	uint32_t c0 = (uint32_t)c_hi;
	uint32_t c1 = (uint32_t)(c_hi >> 32);
	uint64_t p00 = __libc_trig_wide(m0, c0);
	uint64_t p01 = __libc_trig_wide(m0, c1);
	uint64_t p10 = __libc_trig_wide(m1, c0);
	uint64_t middle = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
	uint64_t low = (middle << 32) | (uint32_t)p00;
	uint64_t high = __libc_trig_wide(m1, c1) + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
	uint64_t carry = __libc_trig_wide(m1, (uint32_t)(c_lo >> 32));
	int shift = 52 - e + point - 64;

	/* m c_lo 2^-64, to below a unit of low */
	low += carry;
	high += low < carry;

	if (shift >= 64) {
		*n = 0;
		return high >> (shift - 64);
	}
	*n = (int32_t)(high >> shift);
	return (high << (64 - shift)) | (low >> shift);
}

/* The high 32 bits of a b for signed a and b, from the unsigned product (no __aeabi_lmul on Armv6-M) */
static inline int32_t __libc_dp_fixed_high_signed(int32_t a, int32_t b)
{
	uint32_t high = (uint32_t)(__libc_trig_wide((uint32_t)a, (uint32_t)b) >> 32);

	/* The unsigned product of a two's complement operand counts the other operand 2^32 times too often */
	if (a < 0)
		high -= (uint32_t)b;
	if (b < 0)
		high -= (uint32_t)a;

	return (int32_t)high;
}

/* -t for t = n + f 2^-64 */
static inline uint64_t __libc_dp_fixed_negate(int32_t *n, uint64_t f)
{
	*n = -*n - (f != 0);
	return -f;
}

/*
 * The fast logarithms of the soft-double cores (PLAN D192; docs/research/libm-fast-double.md, section 10): z in
 * [45/64, 90/64) in Q63, the entry by (z - 45/64) in steps of 2^-7, invc in Q31 and -log(invc) in Q64
 * (__libc_dp_fixed_log_table.c)
 */
struct __libc_dp_fixed_log_entry {
	int64_t logc;
	uint32_t invc;
	/* -log(invc) - logc in Q96 (pow's, D194; in what would be the entry's padding) */
	int32_t tail;
};

extern const struct __libc_dp_fixed_log_entry __libc_dp_fixed_log_table[90];

/* ln z in Q64 for z in [45/64, 90/64) in Q63, to a few units (__libc_dp_fixed_log_core.c) */
int64_t __libc_dp_fixed_log_core(uint64_t z);

/*
 * log1p(+-s 2^(*e - 63)) for s normalized (bit 63 set) and a value below 2^-5, relative: the result's significand,
 * normalized, its exponent in *e (__libc_dp_fixed_log_near.c)
 */
uint64_t __libc_dp_fixed_log_near(uint64_t s, int32_t *e, bool negative);

/* The double (n 2^-63) 2^e with sign, n normalized, rounded to nearest by its 54th bit (e within the normal range) */
static inline double __libc_dp_fixed_double(uint64_t sign, int32_t e, uint64_t n)
{
	return __libc_double_from(sign | ((((uint64_t)(int64_t)(e + 1022) << 52) + (n >> 11) + ((n >> 10) & 1))));
}

/* n c 2^-64 for n normalized and c of bit 63 set, renormalized: the significand, and 1 or 0 added to the exponent */
static inline uint64_t __libc_dp_fixed_scale(uint64_t n, uint64_t c, int32_t *e)
{
	uint64_t h = __libc_trig_high64(n, c);

	if ((h >> 63) != 0) {
		*e += 1;
		return h;
	}

	return h << 1;
}

/* |k| c as a 128-bit (hi, lo) for |k| < 2^32 */
static inline void __libc_dp_fixed_k_product(uint32_t k, uint64_t c, uint64_t *hi, uint64_t *lo)
{
	uint64_t p0 = __libc_trig_wide(k, (uint32_t)c);
	uint64_t p1 = __libc_trig_wide(k, (uint32_t)(c >> 32));
	uint64_t middle = (p0 >> 32) + (uint32_t)p1;

	*lo = (middle << 32) | (uint32_t)p0;
	*hi = (p1 >> 32) + (middle >> 32);
}

/*
 * The double of the signed 128-bit value (hi, lo) 2^-64, at least 2^-7 in magnitude (the table path): negated to a
 * magnitude, its top 64 bits normalized and rounded once
 */
static inline double __libc_dp_fixed_from128(uint64_t hi, uint64_t lo)
{
	uint64_t sign = hi & 0x8000000000000000ull;
	int32_t shift;

	if (sign != 0) {
		hi = ~hi + (lo == 0);
		lo = -lo;
	}
	if (hi != 0) {
		shift = 64 - __builtin_clzll(hi);
		return __libc_dp_fixed_double(sign, shift - 1, (hi << (64 - shift)) | (lo >> shift));
	}
	shift = __builtin_clzll(lo);

	return __libc_dp_fixed_double(sign, -1 - shift, lo << shift);
}

/* (hi, lo) += t sign-extended */
static inline void __libc_dp_fixed_add128(uint64_t *hi, uint64_t *lo, int64_t t)
{
	uint64_t sum = *lo + (uint64_t)t;

	*hi += (sum < *lo) - (t < 0);
	*lo = sum;
}

/* (hi, lo) for k c, k signed, |k| < 2^32 */
static inline void __libc_dp_fixed_k_signed(int32_t k, uint64_t c, uint64_t *hi, uint64_t *lo)
{
	__libc_dp_fixed_k_product(k < 0 ? -(uint32_t)k : (uint32_t)k, c, hi, lo);
	if (k < 0) {
		*hi = ~*hi + (*lo == 0);
		*lo = -*lo;
	}
}

/*
 * A positive finite x's bits as 2^k z with z in [45/64, 90/64) in Q63 (a subnormal normalized first)
 */
static inline uint64_t __libc_dp_fixed_log_reduce(uint64_t bits, int32_t *k)
{
	int32_t e = (int32_t)(bits >> 52) - 1023;
	uint64_t z = ((bits << 12) >> 1) | 0x8000000000000000ull;

	if (e == -1023) {
		int32_t shift = __builtin_clzll(bits << 12) + 1;

		z = bits << (11 + shift);
		e = -1022 - shift;
	}
	if (z >= 0xb400000000000000ull) {
		z >>= 1;
		e++;
	}
	*k = e;

	return z;
}

/*
 * The double nearest D136's fixed-point result, (-1)^negative v 2^(exponent - 63), by its 54th bit with no rounding test
 * (D197): the kernel's error, below 18 units of v's last bit (56 for tan), is at most 0.03 ulp more. The kernel's results
 * are normal doubles for 2^-27 <= |x| < 2^20.
 */
static inline double __libc_dp_fixed_trig_round(const struct __libc_trig_fixed *value)
{
	return __libc_dp_fixed_double(value->negative ? 0x8000000000000000ull : 0, value->exponent, value->v);
}

/* atan(j/16) for j = 0..16 in Q64, rounded (libc/math/dp-fixed/fast/__libc_dp_fixed_atan_table.c, D199) */
extern const uint64_t __libc_dp_fixed_atan_table[17];

/*
 * atan(y/x) for 0 < y <= x as significands my, mx (53 bits, bit 52 set) and exponents ey, ex, at most 60 apart (D199;
 * docs/research/libm-fast-double.md, section 17), the quadrant's offset applied: 0 for K, 1 for pi/2 - K, 2 for pi/2 + K,
 * 3 for pi - K; negated when negative; one rounding to nearest (__libc_dp_fixed_atan.c)
 */
double __libc_dp_fixed_atan(uint64_t my, int32_t ey, uint64_t mx, int32_t ex, unsigned int quadrant, bool negative);

#endif /* _DP_FIXED_FAST_H */
