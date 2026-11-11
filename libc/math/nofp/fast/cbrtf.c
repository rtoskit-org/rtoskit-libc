/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cbrtf.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast cbrtf of the cores without an FPU (PLAN D162, D174), in integers: |x| = 2^(3q + j) f with f in [1, 2) and
 * j in {0, 1, 2}, M = 2^j f in Q28; w ~ M^(-1/3) from a degree 2 polynomial in f (2^-8.9) times 2^(-j/3), then two Newton
 * steps w += w (1 - M w^3) / 3, multiplications only (quadratic, to the 2^-31 of Q31), and cbrt M = M w^2 as a 64-bit
 * product, rounded once: below 1 ulp to nearest (the note, section 17). Special values as C23 F.10.4.1.
 */
float cbrtf(float x)
{
	/* 2^(-j/3) in Q31 */
	static const uint32_t scales[3] = { 0x80000000, 0x6597fa95, 0x50a28be6 };
	uint32_t bits = __libc_float_bits(x);
	uint32_t magnitude = bits & 0x7fffffff;
	uint32_t significand;
	uint32_t m;
	uint32_t w;
	uint32_t square;
	int32_t f;
	int32_t p;
	uint64_t y;
	int e;
	int q;
	int j;
	int k;

	if (magnitude == 0 || magnitude >= 0x7f800000)
		return x + x;

	/* A subnormal x is normalised */
	e = (int)(magnitude >> 23);
	significand = magnitude & __LIBC_FLOAT_FRACTION;
	if (e == 0) {
		k = (int)stdc_leading_zeros(significand) - 8;
		significand <<= k;
		e = 1 - k;
	} else {
		significand |= 0x00800000;
	}
	e -= 127;
	/* floor(e / 3) for e in [-149, 127] without a division (Armv6-M has none): n 21846 / 2^16 is floor(n / 3) below 300 */
	q = (int)(((uint32_t)(e + 150) * 21846) >> 16) - 50;
	j = e - 3 * q;
	m = significand << (5 + j);

	/* f in Q30, the polynomial in Q29, then w in Q31 (at most 1) */
	f = (int32_t)(significand << 7);
	p = 0x02eb9c8b;
	p = (int32_t)0xf0bdb5ba + (__libc_fixed_high_signed(f, p) << 2);
	p = 0x2c45ae35 + (__libc_fixed_high_signed(f, p) << 2);
	w = (uint32_t)(__libc_fixed_wide((uint32_t)p << 2, scales[j]) >> 31);

	for (int i = 0; i < 2; ++i) {
		uint32_t w2 = (uint32_t)(__libc_fixed_wide(w, w) >> 31);
		uint32_t w3 = (uint32_t)(__libc_fixed_wide(w2, w) >> 31);
		/* 1 - M w^3, small, in signed Q31 */
		int32_t d = (int32_t)(0x80000000u - (uint32_t)(__libc_fixed_wide(m, w3) >> 28));
		/* w / 3 in Q31 */
		int32_t third = (int32_t)(__libc_fixed_wide(w, 0x55555555) >> 32);

		w += (uint32_t)(__libc_fixed_high_signed(third, d) << 1);
	}

	/* M w^2 in Q59, about 2^59 (cbrt M in [1, 2)): normalised by its leading zeros */
	square = (uint32_t)(__libc_fixed_wide(w, w) >> 31);
	y = __libc_fixed_wide(m, square);
	k = (int)stdc_leading_zeros(y);

	return __libc_fixed_float(y << k, q + 4 - k, (bits >> 31) != 0);
}
