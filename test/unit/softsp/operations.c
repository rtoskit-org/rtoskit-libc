/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * operations.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <complex.h>
#include <stdint.h>
#include <string.h>

#include "internal/aeabi.h"
#include "softsp.h"

/*
 * Every helper of the soft-sp layer's groups (PLAN D109,
 * docs/research/softfloat-thumb2.md), bit for bit, a NaN result needing
 * only to be a quiet NaN:
 * - addition, subtraction, multiplication and division against the
 *   double operation on the widened operands rounded to float, which is
 *   the correctly rounded float result (53 >= 2 * 24 + 2: Figueroa; Roux);
 * - the comparisons against the double comparison, the flag-returning
 *   forms in cfcmp.c;
 * - the conversions from 32-bit integers through double (exact there),
 *   from 64-bit integers against an integer model.
 * The operands: random bit patterns, close exponents that carry or cancel,
 * ties, subnormals, products and quotients at both ends of the range, and
 * every pair of special values. Nothing here computes in float.
 */

static float from(uint32_t bits)
{
	float x;

	memcpy(&x, &bits, sizeof(x));
	return x;
}

static uint32_t bits_of(float x)
{
	uint32_t bits;

	memcpy(&bits, &x, sizeof(bits));
	return bits;
}

static bool is_nan(uint32_t bits)
{
	return (bits << 1) > 0xff000000u;
}

static void check(const char *what, uint32_t a, uint32_t b, uint32_t got, uint32_t want)
{
	if (got == want)
		return;
	if (is_nan(want) && is_nan(got) && (got & 0x00400000u) != 0)
		return;
	fail(what, a, b);
}

/* The double operations, rounded to float: through volatile doubles so the compiler cannot narrow them to float */
static uint32_t ref_add(uint32_t a, uint32_t b)
{
	volatile double x = from(a);
	volatile double y = from(b);

	return bits_of((float)(x + y));
}

static uint32_t ref_sub(uint32_t a, uint32_t b)
{
	volatile double x = from(a);
	volatile double y = from(b);

	return bits_of((float)(x - y));
}

static uint32_t ref_mul(uint32_t a, uint32_t b)
{
	volatile double x = from(a);
	volatile double y = from(b);

	return bits_of((float)(x * y));
}

static uint32_t ref_div(uint32_t a, uint32_t b)
{
	volatile double x = from(a);
	volatile double y = from(b);

	return bits_of((float)(x / y));
}

static void check_compare(uint32_t a, uint32_t b)
{
	volatile double x = from(a);
	volatile double y = from(b);

	check("fcmpeq", a, b, (uint32_t)__aeabi_fcmpeq(from(a), from(b)), x == y);
	check("fcmplt", a, b, (uint32_t)__aeabi_fcmplt(from(a), from(b)), x < y);
	check("fcmple", a, b, (uint32_t)__aeabi_fcmple(from(a), from(b)), x <= y);
	check("fcmpge", a, b, (uint32_t)__aeabi_fcmpge(from(a), from(b)), x >= y);
	check("fcmpgt", a, b, (uint32_t)__aeabi_fcmpgt(from(a), from(b)), x > y);
	check_flag_compare(a, b);
}

static void check_pair(uint32_t a, uint32_t b)
{
	check("fadd", a, b, bits_of(__aeabi_fadd(from(a), from(b))), ref_add(a, b));
	check("fsub", a, b, bits_of(__aeabi_fsub(from(a), from(b))), ref_sub(a, b));
	check("frsub", a, b, bits_of(__aeabi_frsub(from(a), from(b))), ref_sub(b, a));
	check("fmul", a, b, bits_of(__aeabi_fmul(from(a), from(b))), ref_mul(a, b));
	check("fdiv", a, b, bits_of(__aeabi_fdiv(from(a), from(b))), ref_div(a, b));
}

/* The float nearest a 64-bit magnitude, ties to even, computed in integers */
static uint32_t ref_from_u64(uint32_t sign, uint64_t magnitude)
{
	int top = 63;
	uint64_t significand;

	if (magnitude == 0)
		return sign;
	while ((magnitude >> top) == 0)
		--top;
	if (top <= 23) {
		significand = magnitude << (23 - top);
	} else {
		int shift = top - 23;
		uint64_t rest = magnitude & ((1ULL << shift) - 1);
		uint64_t half = 1ULL << (shift - 1);

		significand = magnitude >> shift;
		if (rest > half || (rest == half && (significand & 1) != 0))
			++significand;
		if (significand == (1ULL << 24)) {
			significand >>= 1;
			++top;
		}
	}

	return sign | ((uint32_t)(top + 127) << 23) | ((uint32_t)significand & 0x007fffffu);
}

static void check_conversions(uint64_t x)
{
	uint32_t low = (uint32_t)x;
	int32_t signed_low = (int32_t)low;
	int64_t signed_x = (int64_t)x;
	volatile double wide_u = low;
	volatile double wide_s = signed_low;

	check("ui2f", low, 0, bits_of(__aeabi_ui2f(low)), bits_of((float)wide_u));
	check("i2f", low, 0, bits_of(__aeabi_i2f(signed_low)), bits_of((float)wide_s));
	check("ul2f", (uint32_t)(x >> 32), low, bits_of(__aeabi_ul2f(x)), ref_from_u64(0, x));
	check("l2f", (uint32_t)(x >> 32), low, bits_of(__aeabi_l2f(signed_x)), signed_x < 0 ? ref_from_u64(0x80000000u, 0 - x) : ref_from_u64(0, x));
}

/* A random float with the given exponent field, its sign and fraction random */
static uint32_t with_exponent(uint64_t *state, int field)
{
	uint32_t bits = (uint32_t)next(state);

	if (field < 0)
		field = 0;
	if (field > 254)
		field = 254;
	return (bits & 0x807fffffu) | ((uint32_t)field << 23);
}

static const uint32_t specials[] = {
	0x00000000u,
	0x80000000u,
	0x00000001u,
	0x80000001u,
	0x007fffffu,
	0x807fffffu,
	0x00800000u,
	0x80800000u,
	0x3f800000u,
	0xbf800000u,
	0x3fffffffu,
	0x7f7fffffu,
	0xff7fffffu,
	0x7f800000u,
	0xff800000u,
	0x7fc00000u,
	0xffc00001u,
	0x7f800001u,
	0x34000000u,
	0x33800000u,
	0x4b800000u,
	0x00400000u,
};

static void check_mulsc3_powisf2(void)
{
	float complex product = __mulsc3(1.0f, 2.0f, 3.0f, 4.0f);
	float complex infinite = __mulsc3(__builtin_inff(), __builtin_nanf(""), 1.0f, 0.0f);

	check("mulsc3 real", 0, 0, bits_of(crealf(product)), 0xc0a00000u);
	check("mulsc3 imaginary", 0, 0, bits_of(cimagf(product)), 0x41200000u);
	check("mulsc3 recovery", 0, 0, (bits_of(crealf(infinite)) << 1) == 0xff000000u || (bits_of(cimagf(infinite)) << 1) == 0xff000000u, 1);
	check("powisf2 2^10", 0, 0, bits_of(__powisf2(2.0f, 10)), 0x44800000u);
	check("powisf2 2^-2", 0, 0, bits_of(__powisf2(2.0f, -2)), 0x3e800000u);
	check("powisf2 x^0", 0, 0, bits_of(__powisf2(3.0f, 0)), 0x3f800000u);
}

void test_operations(void)
{
	uint64_t state = 0x2545f4914f6cdd1dULL;
	int count = (int)(sizeof(specials) / sizeof(specials[0]));

	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < count; ++j) {
			check_pair(specials[i], specials[j]);
			check_compare(specials[i], specials[j]);
		}
	}

	for (int i = 0; i < 200000; ++i) {
		uint32_t a = (uint32_t)next(&state);
		uint32_t b = (uint32_t)next(&state);
		int ea = (int)((a >> 23) & 0xff);
		uint32_t r = (uint32_t)next(&state);

		switch (i % 8) {
			case 0:
				/* Random bit patterns */
				break;
			case 1:
				/* Close exponents: carries, cancellation, sticky shifts */
				b = with_exponent(&state, ea + (int)(r % 61) - 30);
				break;
			case 2:
				/* Equal exponents, opposite signs: cancellation down to subnormals */
				b = (a ^ 0x80000000u) ^ (r & 0x0000ffffu);
				break;
			case 3:
				/* A power of two half an ulp below a: ties for the addition */
				b = (r & 0x80000000u) | ((uint32_t)(ea > 25 ? ea - 25 + (int)(r & 1) : 0) << 23);
				break;
			case 4:
				/* Subnormals on one or both sides */
				a = with_exponent(&state, (r & 1) != 0 ? 0 : ea);
				b = with_exponent(&state, 0);
				break;
			case 5:
				/* Products and quotients near the top of the range */
				a = with_exponent(&state, 190 + (int)(r % 64));
				b = with_exponent(&state, 64 + (int)((r >> 8) % 128));
				break;
			case 6:
				/* Products near the subnormal range, quotients near the top */
				a = with_exponent(&state, (int)(r % 64));
				b = with_exponent(&state, 40 + (int)((r >> 8) % 70));
				break;
			default:
				/* Products and quotients whose exponents straddle the subnormal boundary */
				a = with_exponent(&state, 1 + (int)(r % 40));
				b = with_exponent(&state, 100 + (int)((r >> 8) % 60));
				break;
		}
		if (ea == 255 && (i % 8) != 0)
			a &= 0xbfffffffu;
		check_pair(a, b);
		check_pair(b, a);
		if ((i % 4) == 0)
			check_compare(a, b);
	}

	for (int i = 0; i < 100000; ++i) {
		uint64_t x = next(&state);

		check_conversions(x >> (next(&state) % 64));
		check_conversions(x);
	}
	check_conversions(0);
	check_conversions(0x80000000u);
	check_conversions(0x8000000000000000ULL);
	check_conversions(UINT64_MAX);
	check_conversions(0x00ffffffu);
	check_conversions(0x01000001u);
	check_conversions(0x01000003u);

	check_mulsc3_powisf2();
}
