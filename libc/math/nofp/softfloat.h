/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * softfloat.h
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SOFTFLOAT_H
#define _SOFTFLOAT_H

#include <stdbit.h>
#include <stdint.h>

/*
 * Single-precision arithmetic in integers for the cores without an FPU
 * (PLAN D85, docs/research/softfloat.md): round to nearest, ties to even,
 * the only direction <fenv.h> has there (D44), and no exception flags.
 * Nothing in here may compute in float, or the helpers would call
 * themselves.
 */

#define __LIBC_SOFTFLOAT_SIGN 0x80000000u
#define __LIBC_SOFTFLOAT_INFINITY 0x7f800000u
#define __LIBC_SOFTFLOAT_QUIET 0x00400000u
#define __LIBC_SOFTFLOAT_DEFAULT_NAN 0x7fc00000u

static inline uint32_t __libc_softfloat_bits(float x)
{
	uint32_t bits;

	__builtin_memcpy(&bits, &x, sizeof(bits));
	return bits;
}

static inline float __libc_softfloat_from(uint32_t bits)
{
	float x;

	__builtin_memcpy(&x, &bits, sizeof(x));
	return x;
}

/*
 * The 24-bit significand of an operand with its biased exponent, a
 * subnormal one normalised (its exponent then 0 or below): the value is
 * significand 2^(exponent - 150), with the significand in [2^23, 2^24).
 */
static inline uint32_t __libc_softfloat_unpack(uint32_t bits, int *exponent)
{
	uint32_t significand = bits & 0x007fffffu;
	int field = (int)((bits >> 23) & 0xff);

	if (field != 0) {
		*exponent = field;
		return significand | 0x00800000u;
	}

	int shift = (int)stdc_leading_zeros(significand) - 8;

	*exponent = 1 - shift;
	return significand << shift;
}

/*
 * The float nearest x 2^(exponent - 152), ties to even, with the sign: x
 * holds the significand above two extra bits, the first half an ulp (the
 * guard) and the second sticky (anything below it), with x below 2^26.
 * A result below the normal range is shifted first, so it is rounded only
 * once, on the subnormal grid; a carry out of the significand moves to the
 * next exponent through the addition.
 */
static inline uint32_t __libc_softfloat_pack(uint32_t sign, int exponent, uint32_t x)
{
	uint32_t significand;
	uint32_t bits;

	if (exponent >= 255)
		return sign | __LIBC_SOFTFLOAT_INFINITY;
	if (exponent <= 0) {
		unsigned int shift = (unsigned int)(1 - exponent);

		x = shift >= 26 ? x != 0 : (x >> shift) | ((x & ((1u << shift) - 1)) != 0);
		exponent = 1;
	}

	significand = x >> 2;
	if ((x & 2) != 0 && (x & 5) != 0)
		++significand;

	bits = ((uint32_t)(exponent - 1) << 23) + significand;
	if (bits >= __LIBC_SOFTFLOAT_INFINITY)
		return sign | __LIBC_SOFTFLOAT_INFINITY;

	return sign | bits;
}

/*
 * The float nearest a 32-bit magnitude, with the sign (PLAN D109): the
 * leading one moved to bit 31, then its 24 significant bits above a guard
 * bit (bit 7) and a sticky bit for bits 6-0, the layout
 * __libc_softfloat_pack rounds. A magnitude m 2^-k after the shift is
 * that layout x times 2^(6 - k), so the exponent is 158 - k.
 */
static inline uint32_t __libc_softfloat_from_u32(uint32_t sign, uint32_t magnitude)
{
	if (magnitude == 0)
		return sign;

	int shift = (int)stdc_leading_zeros(magnitude);
	uint32_t normal = magnitude << shift;
	uint32_t x = (normal >> 7) << 1 | ((normal & 0x7f) != 0);

	return __libc_softfloat_pack(sign, 158 - shift, x);
}

/*
 * The float nearest a 64-bit magnitude, with the sign (PLAN D109): the
 * leading one moved to bit 63, the low word folded into a sticky bit 0 of
 * the high word (below the guard bit), and the high word rounded as
 * above, 32 places higher: the exponent is 190 - k.
 */
static inline uint32_t __libc_softfloat_from_u64(uint32_t sign, uint64_t magnitude)
{
	if (magnitude == 0)
		return sign;

	int shift = (int)stdc_leading_zeros(magnitude);
	uint64_t normal = magnitude << shift;
	uint32_t high = (uint32_t)(normal >> 32) | ((uint32_t)normal != 0);
	uint32_t x = (high >> 7) << 1 | ((high & 0x7f) != 0);

	return __libc_softfloat_pack(sign, 190 - shift, x);
}

/*
 * The compiler's own helpers whose libgcc versions call the float helpers
 * from inside libgcc, where the linker would take libgcc's grouped float
 * objects to resolve them on armv7-m (PLAN D100, D109); their prototypes
 * are the compiler's
 */
_Complex float __mulsc3(float a, float b, float c, float d);
float __powisf2(float x, int n);

#endif /* _SOFTFLOAT_H */
