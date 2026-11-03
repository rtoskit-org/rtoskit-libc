/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_trig_fixed.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "trig-fixed.h"

/* 2/pi to 192 bits, most significant word first (floor(2/pi 2^192)) */
static const uint32_t two_over_pi[6] = { 0xa2f9836e, 0x4e441529, 0xfc2757d1, 0xf534ddc0, 0xdb629599, 0x3c439041 };

/* pi/2 in units of 2^-63, rounded */
static const uint64_t pi_over_2 = 0xc90fdaa22168c235ULL;

/* 1/(2j+1)! and 1/(2j)! for j = 1 .. 9 in units of 2^-64, rounded: the Taylor coefficients of sin r / r and cos r */
static const uint64_t sine_terms[9] = { 0x2aaaaaaaaaaaaaabULL, 0x0222222222222222ULL, 0x000d00d00d00d00dULL, 0x00002e3bc74aad8eULL, 0x0000006b99159fd5ULL, 0x00000000b092309dULL, 0x0000000000d73f9fULL, 0x000000000000ca96ULL, 0x0000000000000098ULL };
static const uint64_t cosine_terms[9] = { 0x8000000000000000ULL, 0x0aaaaaaaaaaaaaabULL, 0x005b05b05b05b05bULL, 0x0001a01a01a01a02ULL, 0x0000049f93edde28ULL, 0x00000008f76c77fcULL, 0x000000000c9cba54ULL, 0x00000000000d73faULL, 0x0000000000000b41ULL };

/*
 * The library builds with -fno-builtin, so the bit copies and leading-zero
 * counts are spelled as builtins to stay inline (as __libc_sinf_fixed does)
 */

/* The high 64 bits of a b */
static uint64_t high64(uint64_t a, uint64_t b)
{
	uint32_t a0 = (uint32_t)a;
	uint32_t a1 = (uint32_t)(a >> 32);
	uint32_t b0 = (uint32_t)b;
	uint32_t b1 = (uint32_t)(b >> 32);
	uint64_t low;

	/* Horner's tail coefficients fit in 32 bits: two products */
	if (b1 == 0)
		return (__libc_trig_wide(a1, b0) + (__libc_trig_wide(a0, b0) >> 32)) >> 32;

	low = __libc_trig_wide(a0, b0);
	uint64_t cross0 = __libc_trig_wide(a0, b1);
	uint64_t cross1 = __libc_trig_wide(a1, b0);
	uint64_t middle = (low >> 32) + (uint32_t)cross0 + (uint32_t)cross1;

	return __libc_trig_wide(a1, b1) + (cross0 >> 32) + (cross1 >> 32) + (middle >> 32);
}

/* Bits [position, position + 32) of a little-endian array of words, position >= 0 */
static uint32_t word_at(const uint32_t *words, int position)
{
	int index = position / 32;
	int shift = position % 32;

	if (shift == 0)
		return words[index];
	return (words[index] >> shift) | (words[index + 1] << (32 - shift));
}

/*
 * |x| = q pi/2 + r, |r| <= pi/4: q mod 4, and r = (-1)^negative R
 * 2^(exponent - 63) with R's bit 63 set. |x| = m 2^e with m a 53-bit
 * integer; |x| 2/pi = m T 2^(e - 192) with T = floor(2/pi 2^192), whose
 * product (at most 245 bits) holds the integer part from bit 192 - e and
 * 128 fraction bits below it; T's truncation adds less than 2^-150 to the
 * fraction. The nearest integer gives q, and the signed fraction f, |f| <=
 * 1/2, normalised to 64 bits, times pi/2 gives r.
 */
static bool reduce(uint64_t m, int e, int *quadrant, uint64_t *r, int *exponent, bool *negative)
{
	uint32_t m_words[2] = { (uint32_t)m, (uint32_t)(m >> 32) };
	uint32_t product[9] = { 0 };
	uint32_t fraction[5];
	int point = 192 - e;
	int zeros;
	uint64_t f;
	uint64_t reduced;

	/* m T, schoolbook, little-endian words; T's least significant word is two_over_pi[5] */
	for (int i = 0; i < 6; ++i) {
		uint64_t carry = 0;

		for (int j = 0; j < 2; ++j) {
			uint64_t sum = __libc_trig_wide(two_over_pi[5 - i], m_words[j]) + product[i + j] + carry;

			product[i + j] = (uint32_t)sum;
			carry = sum >> 32;
		}
		product[i + 2] = (uint32_t)carry;
	}

	/* The 128 fraction bits below the point, least significant word first, and the two integer bits above it */
	for (int i = 0; i < 4; ++i)
		fraction[i] = word_at(product, point - 128 + 32 * i);
	*quadrant = (int)(word_at(product, point) & 3);

	/* A fraction of 1/2 or more rounds up: take 1 - fraction, negative */
	*negative = (fraction[3] >> 31) != 0;
	if (*negative) {
		uint32_t borrow = 0;

		*quadrant = (*quadrant + 1) & 3;
		for (int i = 0; i < 4; ++i) {
			uint32_t word = fraction[i];

			fraction[i] = 0u - word - borrow;
			borrow = (word | borrow) != 0;
		}
	}

	/* Normalise: 64 significant bits must remain below the leading one */
	fraction[4] = 0;
	zeros = 0;
	while (zeros < 64 && fraction[3 - zeros / 32] == 0)
		zeros += 32;
	if (zeros == 64)
		return false;
	zeros += __builtin_clz(fraction[3 - zeros / 32]);
	if (zeros > 64)
		return false;
	f = ((uint64_t)word_at(fraction, 64 - zeros + 32) << 32) | word_at(fraction, 64 - zeros);

	/* r = f pi/2 = f 2^(-64-zeros) pi_over_2 2^-63 = high64(f, pi_over_2) 2^(-63-zeros) */
	reduced = high64(f, pi_over_2);
	*exponent = -zeros;
	if ((reduced >> 63) == 0) {
		reduced <<= 1;
		*exponent -= 1;
	}
	*r = reduced;
	return true;
}

bool __libc_trig_fixed(double x, unsigned int want, struct __libc_trig_fixed *sine, struct __libc_trig_fixed *cosine)
{
	uint64_t bits;
	uint64_t m;
	int e;
	int quadrant;
	uint64_t r;
	int exponent;
	bool negative;
	bool x_negative;
	uint64_t z;
	int shift;
	struct __libc_trig_fixed sin_r = { 0 };
	struct __libc_trig_fixed cos_r = { 0 };
	bool odd;

	__builtin_memcpy(&bits, &x, sizeof(bits));
	x_negative = (bits >> 63) != 0;
	e = (int)((bits >> 52) & 0x7ff) - 1075;
	m = (bits & 0x000fffffffffffffULL) | 0x0010000000000000ULL;

	/* |x| >= 2^20 (e >= -32): Payne-Hanek's range, left to the double-double path */
	if (e >= -32)
		return false;

	/* Below pi/4 (0x3fe921fb54442d18) r is |x| itself */
	if ((bits & 0x7fffffffffffffffULL) <= 0x3fe921fb54442d18ULL) {
		quadrant = 0;
		negative = false;
		r = m << 11;
		exponent = e + 52;
	} else if (!reduce(m, e, &quadrant, &r, &exponent, &negative)) {
		return false;
	}

	/* z = r^2 in units of 2^-64: R^2 2^(2 exponent - 126) = high64(R, R) 2^(2 exponent + 2) 2^-64, exponent <= -1 */
	shift = -(2 * exponent + 2);
	z = shift >= 64 ? 0 : high64(r, r) >> shift;

	/* An odd quadrant swaps the two: sin x needs cos r there, and cos x sin r */
	odd = (quadrant & 1) != 0;

	/* sin r = r (1 - z (1/3! - z (1/5! - ...))), the bracket in [0.9, 1] in units of 2^-63 */
	if (want & (odd ? __LIBC_TRIG_FIXED_COS : __LIBC_TRIG_FIXED_SIN)) {
		uint64_t t = sine_terms[8];
		uint64_t s63;
		uint64_t p;
		int k;

		for (int j = 7; j >= 0; --j)
			t = sine_terms[j] - high64(z, t);
		s63 = 0x8000000000000000ULL - (high64(z, t) >> 1);
		/* R S 2^(exponent - 126) = high64(R, S) 2^(exponent - 62) */
		p = high64(r, s63);
		k = __builtin_clzll(p);
		sin_r = (struct __libc_trig_fixed){ .v = p << k, .exponent = exponent + 1 - k, .negative = negative };
	}
	/* cos r = 1 - z (1/2! - z (1/4! - ...)), in [0.7, 1] in units of 2^-63 */
	if (want & (odd ? __LIBC_TRIG_FIXED_SIN : __LIBC_TRIG_FIXED_COS)) {
		uint64_t u = cosine_terms[8];
		uint64_t c63;
		int k;

		for (int j = 7; j >= 0; --j)
			u = cosine_terms[j] - high64(z, u);
		c63 = 0x8000000000000000ULL - (high64(z, u) >> 1);
		k = __builtin_clzll(c63);
		cos_r = (struct __libc_trig_fixed){ .v = c63 << k, .exponent = -k, .negative = false };
	}

	/* sin(q pi/2 + r) and cos(q pi/2 + r); sin is odd and cos even in x */
	if (want & __LIBC_TRIG_FIXED_SIN) {
		*sine = odd ? cos_r : sin_r;
		sine->negative = (sine->negative != (quadrant >= 2)) != x_negative;
	}
	if (want & __LIBC_TRIG_FIXED_COS) {
		*cosine = odd ? sin_r : cos_r;
		cosine->negative = cosine->negative != (quadrant == 1 || quadrant == 2);
	}
	return true;
}
