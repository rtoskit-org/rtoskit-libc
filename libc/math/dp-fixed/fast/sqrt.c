/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sqrt.c
 *
 * Created on: Nov 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math.h"

/*
 * The midpoint's side, exactly: the root rounds up when (2 q + 1) 2^10, the midpoint above q in units of 2^-64, is below
 * sqrt(t 2^64), i.e. (2 q + 1)^2 < t 2^44. Within 8 units of the midpoint the two differ by at most about 2 2^54 2^-7 =
 * 2^48, so their difference modulo 2^64, as a signed number, has the true sign: the square's low 64 bits only (two 32 x
 * 32 products, m = h 2^32 + l, l^2 + 2 h l 2^32), never equal to t 2^44's (the square is odd)
 */
static inline uint64_t sqrt_decide(uint64_t q, uint64_t t)
{
	uint64_t m = 2 * q + 1;
	uint32_t l = (uint32_t)m;
	uint64_t low = __libc_trig_wide(l, l) + (__libc_trig_wide((uint32_t)(m >> 32), l) << 33);

	return (int64_t)((t << 44) - low) > 0 ? q + 1 : q;
}

/* The ends (C23 7.12.7.10, F.10.4.10): NaN returns itself, a negative x (not -0) is invalid, +-0 and +inf return themselves */
[[gnu::noinline]] static double sqrt_ends(double x)
{
	if (isnan(x))
		return x + x;
	if (x < 0)
		return __libc_math_invalid();

	return x;
}

/*
 * The fast sqrt of the soft-double cores (PLAN D162, D219, D233), correctly rounded in 64-bit integers with no
 * soft-double operation (docs/research/libm-fast-double.md, section 36): x = m 2^(e - 52), m normalized (subnormals
 * too); t = m 2^10 or m 2^11 for an even or odd e, so that T = t 2^-64 is x/4^k in [1/4, 1); s = sqrt(T) 2^64 by
 * __libc_dp_fixed_root (the rsqrt seed, three Newton steps and two remainder corrections; within about 6 units), q its
 * top 53 bits; the 11 bits below decide the rounding, or within 8 units of the midpoint the exact test sqrt_decide.
 * These cores round to nearest and raise no flags (D44). C23 7.12.7.10.
 */
double sqrt(double x)
{
	uint64_t bits = __libc_double_bits(x);
	int32_t e = (int32_t)(bits >> 52) - 1023;
	uint64_t m = bits & 0x000fffffffffffffull;
	uint64_t t;
	uint64_t s;
	uint64_t q;
	uint64_t r;

	/* Positive normal and subnormal x only: bits in (0, +inf) */
	if (bits - 1 >= 0x7ff0000000000000ull - 1)
		return sqrt_ends(x);
	if (e == -1023) {
		int k = __builtin_clzll(m) - 11;

		m <<= k;
		e = -1022 - k;
	} else {
		m |= 0x0010000000000000ull;
	}
	t = (e & 1) != 0 ? m << 11 : m << 10;
	s = __libc_dp_fixed_root(t);
	/* T from 1/4: a root within its error below 2^63 is 2^63 (sqrt(1/4) exactly, or rounding to it) */
	if (s < 0x8000000000000000ull)
		s = 0x8000000000000000ull;
	q = s >> 11;
	r = s & 0x7ff;
	if (r - 0x400 + 8 <= 16)
		q = sqrt_decide(q, t);
	else
		q += r > 0x400;

	/* sqrt(x) = 2 sqrt(T) 2^((e - odd)/2) = q 2^(E - 52), E = floor(e/2); q may reach 2^53, carrying into the exponent */
	return __libc_double_from(((uint64_t)(int64_t)((e >> 1) + 1022) << 52) + q);
}
