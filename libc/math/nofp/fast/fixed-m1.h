/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fixed-m1.h
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FIXED_M1_H
#define _FIXED_M1_H

#include <stdbit.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"

/*
 * The fixed-point m1 forms of the cores without an FPU (the fast flavour, PLAN D173; docs/research/libm-fast-float-exp-log.md
 * section 16): e^u - 1 near 0 by a series, 2^t - 1 elsewhere from the exp2 core, both rounded once to float
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
 * e^u - 1 for 2^-60 <= |u| < 2^-3, u = mu 2^(eu - 31) with bit 31 of mu set: u (1 + u p(u)), p = (e^u - 1 - u)/u^2 by its
 * Taylor series to u^5 (the next term below 2^-33 relative) in signed Q31 by Horner's rule with u in Q34, then mu (1 + u p)
 * as a 64-bit product: the relative error stays about 2^-30 however small u is
 */
static inline float __libc_expm1f_series_fixed(uint32_t mu, int eu, bool negative)
{
	static const int32_t coefficients[6] = { 0x40000000, 0x15555555, 0x05555555, 0x01111111, 0x002d82d8, 0x00068068 };
	int shift = -3 - eu;
	int32_t u = shift < 32 ? (int32_t)(mu >> shift) : 0;
	int32_t p = coefficients[5];
	uint64_t m;
	int k;

	if (negative)
		u = -u;
	for (int k = 4; k >= 0; --k)
		p = coefficients[k] + (__libc_fixed_high_signed(u, p) >> 2);
	/* mu (1 + u p) 2^31: 1 + u p in (0.93, 1.07), so the product is in (0.93 2^62, 1.07 2^63), normalised by its leading zeros */
	m = __libc_fixed_wide(mu, 0x80000000u + (uint32_t)(__libc_fixed_high_signed(u, p) >> 2));
	k = (int)stdc_leading_zeros(m);

	return __libc_fixed_float(m << k, eu + 1 - k, negative);
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

#endif /* _FIXED_M1_H */
