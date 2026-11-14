/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tanh.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math-fast.h"

/* G = 1/3 - (2/15) u + (17/315) u^2 - ..., |coefficient| 64^-j in Q64 for j = 0..8, the coefficients of w^j for w = 64 x^2 (the tenth is below 2^-68) */
#define G0 0x5555555555555555ull
#define G1 0x0088888888888889ull
#define G2 0x0000dd0dd0dd0dd1ull
#define G3 0x000001664f4882c1ull
#define G4 0x0000000244dc6abdull
#define G5 0x0000000003ada7a2ull
#define G6 0x000000000005f68eull
#define G7 0x00000000000009abull
#define G8 0x0000000000000010ull

/* The ends: +-0 and NaN return themselves, +-inf +-1, below 2^-27 x nudged toward zero (underflow for a subnormal), from 22 +-(1 - tiny) */
[[gnu::noinline]] static double tanh_ends(double x)
{
	volatile double tiny = 0x1p-80;

	if (isnan(x) || x == 0)
		return x + x;
	if (isinf(x))
		return __builtin_copysign(1.0, x);
	if (__builtin_fabs(x) < 0x1p-27)
		return __libc_math_tiny(x, -x);

	/* At run time, so that it is inexact and rounds in every direction */
	return __builtin_copysign(1.0 - tiny, x);
}

/*
 * The fast tanh of the soft-double cores (PLAN D162, D202), in 64-bit fixed point with one rounding and no soft-double
 * operation on the common path, 2^-27 <= |x| < 22:
 * - below 1/8, x (1 - V) with V = u G(u), u = x^2, in Q64 from w = 64 u (Horner's rule, unsigned and subtracting: G's
 *   terms alternate and each nested value stays positive), relative however small x is;
 * - from 1/8, (Y - c)/(Y + c) with e^(2|x|) = 2^(n - 63) Y from exp's kernel and c = 2^(63 - n): both exact in integers,
 *   the quotient by the 64-bit reciprocal (__libc_dp_fixed_reciprocal), to a few units of 2^-62 of the result.
 * C23 7.12.5.6, F.10.2.6.
 */
double tanh(double x)
{
	uint64_t bits = __libc_double_bits(x);
	uint64_t sign = bits & 0x8000000000000000ull;
	uint64_t magnitude = bits ^ sign;
	int32_t e = (int32_t)(magnitude >> 52) - 1023;
	uint64_t m = ((magnitude & 0x000fffffffffffffull) | 0x0010000000000000ull) << 11;
	uint64_t num;
	uint64_t den;
	uint64_t c;
	uint64_t w;
	uint64_t g;
	uint64_t y;
	int32_t kd;
	int32_t kn;
	int32_t n;

	/* 2^-27 <= |x| < 22 */
	if (__libc_fast_exp_outside(x, 0x3e4, 0x404) || magnitude >= 0x4036000000000000ull)
		return tanh_ends(x);

	if (e < -3) {
		/* w = 64 x^2 in Q64 from M^2 in Q62 */
		w = __libc_trig_high64(m, m) >> (-2 * e - 8);
		g = G7 - __libc_trig_high64(w, G8);
		g = G6 - __libc_trig_high64(w, g);
		g = G5 - __libc_trig_high64(w, g);
		g = G4 - __libc_trig_high64(w, g);
		g = G3 - __libc_trig_high64(w, g);
		g = G2 - __libc_trig_high64(w, g);
		g = G1 - __libc_trig_high64(w, g);
		g = G0 - __libc_trig_high64(w, g);
		/* V = (w/64) G; M (1 - V) in Q62 */
		return __libc_dp_fixed_normal(sign, e + 1, __libc_trig_high64(m, 0x8000000000000000ull - (__libc_trig_high64(w, g) >> 7)));
	}

	/* 2|x| log2(e) = n + f 2^-64, n <= 63 below 22 */
	y = __libc_dp_fixed_exp2_fraction(__libc_dp_fixed_scaled(m >> 11, e + 1, 0xb8aa3b295c17f0bbull, 0xbe87fed0691d3e88ull, 63, &n));
	c = 0x8000000000000000ull >> n;
	num = y - c;
	/* (Y + c)/2, in [2^62, 2^63 + 2^62), and both normalized */
	den = (y >> 1) + (c >> 1);
	kd = __builtin_clzll(den);
	kn = __builtin_clzll(num);
	/* num/den 2^63 in [2^62, 2^64): (Y - c)/(Y + c) = that 2^-63 2^(kd - kn - 1) */
	return __libc_dp_fixed_normal(sign, kd - kn - 1, __libc_trig_high64(num << kn, __libc_dp_fixed_reciprocal(den << kd)));
}
