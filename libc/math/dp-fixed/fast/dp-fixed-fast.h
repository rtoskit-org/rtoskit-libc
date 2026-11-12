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

/* -t for t = n + f 2^-64 */
static inline uint64_t __libc_dp_fixed_negate(int32_t *n, uint64_t f)
{
	*n = -*n - (f != 0);
	return -f;
}

#endif /* _DP_FIXED_FAST_H */
