/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * powf.c
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "fixed.h"
#include "internal/math.h"
#include "libm-fixed.h"

/* 1 for an odd integer y, 0 for an even one (every |y| from 2^24 up), -1 when y is not an integer */
static int integer_kind(float y)
{
	if (truncf(y) != y)
		return -1;
	if (fabsf(y) >= 0x1p24f)
		return 0;

	return ((int32_t)y & 1) != 0 ? 1 : 0;
}

/*
 * The fixed-point path (PLAN D115): z = y log2 |x| in units of 2^-37 from the
 * 24-bit significand of y times the 64-bit logarithm of __libc_log2f_wide
 * (relative error about 2^-44, so |z| < 2^7 is in error by about 2^-37),
 * then the exp2 core and its rounding test (PLAN D86) for a normal result.
 * false leaves the result to the float-float path.
 */
static bool fixed(uint32_t xbits, uint32_t ybits, bool negative, float *out)
{
	int ey = (int)((ybits >> 23) & 0xff);
	uint32_t my = ybits & 0x007fffff;
	bool zneg;
	int el;
	uint64_t v = __libc_log2f_wide(xbits & 0x7fffffff, &el, &zneg);
	uint64_t a;
	int64_t z;
	int shift;
	int exponent;
	uint32_t f;
	uint32_t result;

	if (ey == 0)
		ey = 1;
	else
		my |= 0x00800000;

	/* z = my v 2^(ey + el - 176) units of 2^-37 (y = my 2^(ey - 150), log2 |x| = v 2^(el - 63)): my v is at least 2^86 for a normal y, so a shift below 43 means |z| >= 2^7, out of the normal range */
	shift = 176 - ey - el;
	if (shift < 43)
		return false;
	a = __libc_fixed_wide(my, (uint32_t)(v >> 32)) + (__libc_fixed_wide(my, (uint32_t)v) >> 32);
	shift -= 32;
	z = shift < 64 ? (int64_t)((a + (UINT64_C(1) << (shift - 1))) >> shift) : 0;
	if (zneg != ((ybits >> 31) != 0))
		z = -z;

	f = __libc_exp2f_fixed(z, &exponent);
	if (exponent < -126 || exponent > 127)
		return false;
	if (!__libc_fixed_round(f, exponent, __LIBC_FIXED_EXP2_ERROR, &result) || result >= 0x7f800000)
		return false;
	*out = __libc_float_from(result | (negative ? 0x80000000u : 0));
	return true;
}

/*
 * C23 7.12.7.5, F.10.4.5: for finite non-zero x other than +-1 and finite
 * non-zero y (an odd integer y gives a negative x's sign), the fixed-point
 * path; otherwise, and when its rounding is not certain, the special cases,
 * then 2^(y log2 |x|) with the logarithm and the product as float-float
 * (PLAN D59).
 */
float powf(float x, float y)
{
	uint32_t xbits = __libc_float_bits(x);
	uint32_t ybits = __libc_float_bits(y);
	int kind;
	bool negative;
	struct __libc_ff log;
	struct __libc_ff z;
	float result;

	if ((xbits & 0x7fffffff) - 1 < 0x7f7fffff && (xbits & 0x7fffffff) != 0x3f800000 && (ybits & 0x7fffffff) - 1 < 0x7f7fffff) {
		kind = (xbits >> 31) != 0 ? integer_kind(y) : 0;
		if (kind >= 0 && fixed(xbits, ybits, kind == 1, &result))
			return result;
	}

	if (y == 0 || x == 1.0f)
		return 1.0f;
	if (isnan(x) || isnan(y))
		return x + y;
	if (isinf(y)) {
		if (fabsf(x) == 1.0f)
			return 1.0f;
		return (fabsf(x) < 1.0f) == (y < 0) ? INFINITY : 0.0f;
	}

	kind = integer_kind(y);
	negative = signbit(x) && kind == 1;
	if (x == 0) {
		/* +-infinity with divide-by-zero for a negative y */
		if (y < 0)
			return (negative ? -1.0f : 1.0f) / fabsf(x);
		return negative ? -0.0f : 0.0f;
	}
	if (isinf(x)) {
		float value = y < 0 ? 0.0f : INFINITY;

		return negative ? -value : value;
	}
	if (x < 0 && kind < 0)
		return __libc_math_invalidf();
	if (y == 1.0f)
		return x;

	log = __libc_log2f_core(fabsf(x), 0.0f);
	/* |log2 |x|| is at least 2^-23 here, so a |y| above 2^31 is far out of range either way */
	if (fabsf(y) > 0x1p31f)
		z = (struct __libc_ff){ .hi = y * log.hi, .lo = 0.0f };
	else
		z = __libc_ff_multiply(log, (struct __libc_ff){ .hi = y, .lo = 0.0f });

	return __libc_exp2f_signed(z, negative);
}
