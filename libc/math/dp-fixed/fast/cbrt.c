/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cbrt.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"

/*
 * The fast cbrt of the soft-double cores (PLAN D162, D195; docs/research/libm-fast-double.md, section 13), in integers:
 * |x| = 2^(3q + j) M with M = 2^j f, f in [1, 2) its 53-bit significand. u ~ f^(-1/3), in (0.79, 1] so that it and its
 * powers keep 31 bits in Q31, from a degree-2 seed (2^-8.9) and two Newton steps u += u (1 - f u^3)/3, multiplications
 * only; T = f u^2 cbrt(2^j) in Q31, about 2^-30 from cbrt M. The residual R = M - T^3 is exact in 96 bits (M has 53 bits,
 * T^3 96), and one Newton step with it, T + R/(3 T^2) with 1/T^2 ~ u^2 2^(-2j/3), leaves about 2^-59 before the one
 * rounding to nearest, the only direction these cores have (their <fenv.h> defines FE_TONEAREST alone, D44).
 * C23 7.12.7.1, F.10.4.1: +-0, +-inf and NaN return themselves.
 */
double cbrt(double x)
{
	/* cbrt(2^j) in Q31, and 2^(-2j/3)/3 in Q32 */
	static const uint32_t roots[3] = { 0x80000000, 0xa14517cc, 0xcb2ff52a };
	static const uint32_t thirds[3] = { 0x55555555, 0x35c1b299, 0x21dd5387 };
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	uint64_t significand;
	uint64_t cube_square;
	uint64_t cube_low;
	uint64_t residual_low;
	uint64_t scaled;
	uint64_t p0;
	uint64_t p1;
	uint64_t r;
	uint64_t s;
	uint32_t f;
	uint32_t u;
	uint32_t square;
	uint32_t t;
	uint32_t c;
	uint32_t cube_high;
	int64_t residual;
	int64_t correction;
	int64_t refined;
	int32_t p;
	int e;
	int q;
	int j;

	if (magnitude == 0 || magnitude >= 0x7ff0000000000000ull)
		return x + x;

	/* A subnormal x is normalised */
	e = (int)(magnitude >> 52);
	significand = magnitude & 0x000fffffffffffffull;
	if (e == 0) {
		int k = __builtin_clzll(significand) - 11;

		significand <<= k;
		e = 1 - k;
	} else {
		significand |= 0x0010000000000000ull;
	}

	/* floor((e - 1023)/3) without a division (Armv6-M has none): n 21846 / 2^16 is floor(n / 3) below 32768 */
	q = (int)(((uint32_t)(e + 54) * 21846u) >> 16) - 359;
	j = e - 1023 - 3 * q;
	/* f in Q31 */
	f = (uint32_t)(significand >> 21);

	/* The seed polynomial in f (Q30) in Q29, then u in Q31 */
	p = 0x02eb9c8b;
	p = (int32_t)0xf0bdb5ba + (__libc_dp_fixed_high_signed((int32_t)(f >> 1), p) << 2);
	p = 0x2c45ae35 + (__libc_dp_fixed_high_signed((int32_t)(f >> 1), p) << 2);
	u = (uint32_t)p << 2;

	for (int i = 0; i < 2; ++i) {
		uint32_t u2 = (uint32_t)(__libc_trig_wide(u, u) >> 31);
		uint32_t u3 = (uint32_t)(__libc_trig_wide(u2, u) >> 31);
		/* 1 - f u^3, small, in signed Q31 */
		int32_t d = (int32_t)(0x80000000u - (uint32_t)(__libc_trig_wide(f, u3) >> 31));
		/* u / 3 in Q31 */
		uint32_t third = (uint32_t)(__libc_trig_wide(u, 0x55555555) >> 32);

		u += (uint32_t)(__libc_dp_fixed_high_signed((int32_t)third, d) << 1);
	}

	/* T = f u^2 cbrt(2^j) in Q31, cbrt M in [1, 2): clamped below 2^32, where cbrt M is within 2^-30 of 2 */
	square = (uint32_t)(__libc_trig_wide(u, u) >> 31);
	s = __libc_trig_wide((uint32_t)(__libc_trig_wide(f, square) >> 31), roots[j]) >> 31;
	t = (s >> 32) != 0 ? 0xffffffffu : (uint32_t)s;

	/* R = M 2^93 - T^3 in units of 2^-93, exactly in 96 bits: M 2^93 is the significand shifted by 41 + j */
	scaled = significand << (41 + j);
	cube_square = __libc_trig_wide(t, t);
	p0 = __libc_trig_wide((uint32_t)cube_square, t);
	p1 = __libc_trig_wide((uint32_t)(cube_square >> 32), t);
	cube_low = p0 + (p1 << 32);
	cube_high = (uint32_t)(p1 >> 32) + (cube_low < p0);
	/* R in units of 2^-64 (|R| below 2^-24): the high word M 2^93 >> 64 is the significand shifted by 23 - j */
	residual_low = scaled - cube_low;
	residual = (int64_t)(((uint64_t)(int64_t)(int32_t)((uint32_t)(significand >> (23 - j)) - cube_high - (scaled < cube_low)) << 35) | (residual_low >> 29));

	/* d = R/(3 T^2) in Q62, |d| below 2^-26: u^2 2^(-2j/3)/3 in Q32, times |R|, the sign after */
	c = (uint32_t)(__libc_trig_wide(square, thirds[j]) >> 31);
	r = residual < 0 ? -(uint64_t)residual : (uint64_t)residual;
	correction = (int64_t)(((__libc_trig_wide((uint32_t)r, c) >> 32) + __libc_trig_wide((uint32_t)(r >> 32), c)) >> 2);
	if (residual < 0)
		correction = -correction;

	/*
	 * T + d is within about 2^-55 of cbrt M (Newton's d^2/T and c's 2^-28), so its residual M - (T + d)^3 = R - d (3 T^2 +
	 * 3 T d + d^2) is below 2^-48: in units of 2^-100 it fits 64 bits, so each term is formed modulo 2^64. R is exact, 3 T^2 d
	 * exact before its floor (T^2 exact in Q62, d in Q62), 3 T d^2 (2^-49) to 2^-12 of itself, d^3 (2^-78) left out.
	 */
	r = correction < 0 ? -(uint64_t)correction : (uint64_t)correction;
	p0 = __libc_trig_wide((uint32_t)cube_square, (uint32_t)r);
	/* T^2 |d| in units of 2^-124 (|d| below 2^37 in Q62), shifted by 32 and modulo 2^56, then its bits 24 and up modulo 2^64 */
	p1 = __libc_trig_wide((uint32_t)(cube_square >> 32), (uint32_t)r) + __libc_trig_wide((uint32_t)cube_square, (uint32_t)(r >> 32)) + (p0 >> 32) + (__libc_trig_wide((uint32_t)(cube_square >> 32), (uint32_t)(r >> 32)) << 32);
	scaled = 3 * ((p1 << 8) | ((uint32_t)p0 >> 24));
	if (correction < 0)
		scaled = -scaled;
	/* 3 T d^2: |d| in Q56 squared (Q112), times T (Q31) in units of 2^-111 */
	p0 = __libc_trig_wide((uint32_t)(r >> 6), (uint32_t)(r >> 6));
	refined = (int64_t)(((residual_low << 7) - scaled - ((3 * __libc_trig_wide((uint32_t)(p0 >> 32), t)) >> 11)));

	/* Plus R'/(3 T^2): R' (below 2^-48) in units of 2^-78 times c, in Q62 */
	s = ((uint64_t)t << 31) + (uint64_t)correction + (uint64_t)(int64_t)(__libc_dp_fixed_high_signed((int32_t)(refined >> 22), (int32_t)c) >> 16);

	/* cbrt M in Q62, normalised to Q63: within 2^-58 of [1, 2), so the leading bit is bit 63, 62 or (below 1) 61 */
	if ((s >> 63) != 0)
		return __libc_dp_fixed_double(sign, q + 1, s);
	if ((s >> 62) != 0)
		return __libc_dp_fixed_double(sign, q, s << 1);

	return __libc_dp_fixed_double(sign, q - 1, s << 2);
}
