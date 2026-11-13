/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_atan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* 2^127 / d for d with bit 63 set, to about 2^-63 relatively: a 17-bit seed by one 32/16-bit division, two Newton steps */
static inline uint64_t reciprocal(uint64_t d)
{
	uint64_t y = (uint64_t)(0xffffffffu / (uint32_t)(d >> 48)) << 47;

	for (int i = 0; i < 2; ++i) {
		/* d y 2^-64 is about 2^63; 2^64 minus it is the factor 2 - d y 2^-127 in units of 2^-63 */
		uint64_t factor = 0 - __libc_trig_high64(d, y);
		uint64_t next = __libc_trig_high64(y, factor);

		y = (next >> 63) != 0 ? UINT64_MAX : next << 1;
	}
	return y;
}

/*
 * atan t = t (1 - V) for |t| <= 1/32 and a little past it (j's estimate): V = u G(u), u = t^2, G = 1/3 - u/5 + u^2/7 - ...
 * to u^5/13 (the next term is below 2^-70), by unsigned Horner's rule in w = 2^8 u (each nested term positive, w below
 * 2^-1 in Q64 even at t = 2^-4.5); V in Q64
 */
static inline uint64_t series(uint64_t w)
{
	uint64_t g = 0x000000001745d174ull - __libc_trig_high64(w, 0x000000000013b13bull);

	g = 0x0000001c71c71c72ull - __libc_trig_high64(w, g);
	g = 0x0000249249249249ull - __libc_trig_high64(w, g);
	g = 0x0033333333333333ull - __libc_trig_high64(w, g);
	g = 0x5555555555555555ull - __libc_trig_high64(w, g);

	return __libc_trig_high64(w, g) >> 8;
}

/* The double of v 2^(e - 63), v nonzero, normalized by its leading zeros, with sign; rounded once */
static inline double result(uint64_t v, int32_t e, bool negative)
{
	int k = __builtin_clzll(v);

	return __libc_dp_fixed_double(negative ? 0x8000000000000000ull : 0, e - k, v << k);
}

/*
 * The kernel (PLAN D199; docs/research/libm-fast-double.md, section 17): j = round(16 y/x) from one 32-bit division of the
 * top bits; for j >= 1 (the exponents at most 6 apart), y and x aligned so that c x and c y are exact (c = j/16), N =
 * y - c x and D = x + c y exactly, |t| = |N|/D in Q64 by a 64-bit reciprocal, K = atan(j/16) +- atan|t| in Q64; for
 * j = 0, t = y/x relative, so tiny ratios keep their accuracy. The offsets pi/2 and pi in Q62.
 */
double __libc_dp_fixed_atan(uint64_t my, int32_t ey, uint64_t mx, int32_t ex, unsigned int quadrant, bool negative)
{
	constexpr uint64_t half_pi = 0x6487ed5110b4611aull;
	constexpr uint64_t pi = 0xc90fdaa22168c235ull;
	int32_t d = ex - ey;
	uint32_t j = 0;
	uint64_t x = mx << 10;
	uint64_t y = 0;
	uint64_t k;

	if (d <= 6) {
		uint32_t den = (uint32_t)(x >> 36);

		y = my << (10 - d);
		j = ((uint32_t)(y >> 32) + (den >> 1)) / den;
	}

	if (j == 0) {
		/* t = y/x as q 2^(e - 63), q normalized: (my 2^11) 2^127/(mx 2^11) 2^-64 is the quotient in (2^62, 2^64) */
		uint64_t q = __libc_trig_high64(my << 11, reciprocal(mx << 11));
		int32_t e = ey - ex;
		int32_t shift;
		uint64_t w;

		if ((q >> 63) == 0) {
			q <<= 1;
			e -= 1;
		}
		/* w = 2^8 t^2 in Q64 = (q^2 2^-64) 2^(2e + 10), e <= -5 here */
		shift = -(2 * e + 10);
		w = shift >= 64 ? 0 : __libc_trig_high64(q, q) >> shift;
		k = q - __libc_trig_high64(q, series(w));
		if (quadrant == 0)
			return result(k, e, negative);
		/* K in Q62: k 2^(e - 63) = (k >> (1 - e)) 2^-62 */
		k = 1 - e >= 64 ? 0 : k >> (1 - e);
	} else {
		/* N = y - c x and D = x + c y exactly (c x = j mx 2^6, c y = j (y >> 4), both exact for d <= 6) */
		int64_t n = (int64_t)(y - (uint64_t)j * (mx << 6));
		uint64_t a = n < 0 ? -(uint64_t)n : (uint64_t)n;
		uint64_t den = x + (uint64_t)j * (y >> 4);
		int s = (den >> 63) != 0 ? 0 : 1;
		/* |t| 2^64 = |N| 2^(64 + s)/(D 2^s) = (|N| 2^(1 + s)) 2^127/(D 2^s) 2^-64 */
		uint64_t t = __libc_trig_high64(a << (1 + s), reciprocal(den << s));
		uint64_t w = __libc_trig_high64(t, t) << 8;
		uint64_t at = t - __libc_trig_high64(t, series(w));

		k = n < 0 ? __libc_dp_fixed_atan_table[j] - at : __libc_dp_fixed_atan_table[j] + at;
		if (quadrant == 0)
			return result(k, -1, negative);
		k >>= 2;
	}

	/* pi/2 - K, pi/2 + K or pi - K in Q62 */
	if (quadrant == 1)
		k = half_pi - k;
	else if (quadrant == 2)
		k = half_pi + k;
	else
		k = pi - k;

	return result(k, 1, negative);
}
