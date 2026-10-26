/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_dense.c
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * Dense sweeps where the speed flavour's fixed-point float functions on
 * Armv6-M (PLAN D86, D87, D88, D114, D115) have their largest error: the logarithms around
 * 1, where the result is the series alone and the neighbouring table
 * entries cancel against it, and the exponentials, the trigonometric and
 * the error functions over their whole fast range.
 * Each result is compared with the double function rounded to float; at most
 * one in ten thousand, plus one, may differ (the float-float paths' own
 * allowance, D57, and the reference's double rounding). A fast path that
 * rounded without its error test fails here. Every board runs the sweeps on
 * the library as built.
 */

static int failures;

static uint32_t bits_of(float x)
{
	uint32_t bits;

	memcpy(&bits, &x, sizeof(bits));
	return bits;
}

static float float_of(uint32_t bits)
{
	float x;

	memcpy(&x, &bits, sizeof(x));
	return x;
}

/* Every stride-th float in [low, high), both signs if asked */
static void sweep(const char *name, float (*function)(float), double (*reference)(double), float low, float high, uint32_t stride, bool both_signs)
{
	unsigned long count = 0;
	unsigned long wrong = 0;

	for (uint32_t bits = bits_of(low); bits < bits_of(high); bits += stride) {
		for (int sign = 0; sign <= (both_signs ? 1 : 0); ++sign) {
			float x = float_of(bits | (sign != 0 ? 0x80000000u : 0));
			float result = function(x);
			float want = (float)reference((double)x);

			++count;
			if (bits_of(result) != bits_of(want)) {
				if (wrong < 3)
					printf("%s(%a) = %a, want %a\n", name, (double)x, (double)result, (double)want);
				++wrong;
			}
		}
	}

	printf("%s: %lu of %lu not correctly rounded\n", name, wrong, count);
	if (wrong > count / 10000 + 1)
		++failures;
}

/* Results near a halfway point must be correctly rounded: a fast path that rounded without its error test fails here */
static const struct {
	float (*function)(float);
	float x;
	float want;
} near[] = {
#include "near.inc"
};

/* The same for the two-argument functions */
static const struct {
	float x;
	float y;
	float want;
} near2[] = {
#include "near2.inc"
};

static void check_near2(void)
{
	int wrong = 0;

	for (unsigned int i = 0; i < sizeof(near2) / sizeof(near2[0]); ++i) {
		float result = powf(near2[i].x, near2[i].y);

		if (bits_of(result) != bits_of(near2[i].want)) {
			printf("near2 %u: powf(%a, %a) = %a, want %a\n", i, (double)near2[i].x, (double)near2[i].y, (double)result, (double)near2[i].want);
			++wrong;
		}
	}

	printf("near2: %d of %u not correctly rounded\n", wrong, (unsigned int)(sizeof(near2) / sizeof(near2[0])));
	if (wrong != 0)
		++failures;
}

static void check_near(void)
{
	int wrong = 0;

	for (unsigned int i = 0; i < sizeof(near) / sizeof(near[0]); ++i) {
		float result = near[i].function(near[i].x);

		if (bits_of(result) != bits_of(near[i].want)) {
			printf("near %u: f(%a) = %a, want %a\n", i, (double)near[i].x, (double)result, (double)near[i].want);
			++wrong;
		}
	}

	printf("near: %d of %u not correctly rounded\n", wrong, (unsigned int)(sizeof(near) / sizeof(near[0])));
	if (wrong != 0)
		++failures;
}

int main(void)
{
	/* Around 1 and across sqrt(2), where the reduction changes binade */
	sweep("logf", logf, log, 0x1.d0p-1f, 0x1.18p0f, 127, false);
	sweep("log2f", log2f, log2, 0x1.d0p-1f, 0x1.18p0f, 127, false);
	sweep("logf", logf, log, 0x1.64p0f, 0x1.70p0f, 31, false);
	sweep("log2f", log2f, log2, 0x1.64p0f, 0x1.70p0f, 31, false);
	sweep("log10f", log10f, log10, 0x1.d0p-1f, 0x1.18p0f, 127, false);
	sweep("log10f", log10f, log10, 0x1.64p0f, 0x1.70p0f, 31, false);
	sweep("log10f", log10f, log10, 0x1p-149f, 0x1.fffffep127f, 1u << 17, false);

	/* The exponentials over one binade of arguments of either sign, and arguments of every binade */
	sweep("expf", expf, exp, 0x1p4f, 0x1p5f, 1021, true);
	sweep("exp2f", exp2f, exp2, 0x1p4f, 0x1p5f, 1021, true);
	sweep("expf", expf, exp, 0x1p-30f, 0x1.5cp6f, 1u << 17, true);
	sweep("exp2f", exp2f, exp2, 0x1p-30f, 0x1.f8p6f, 1u << 17, true);

	/* The trigonometric functions: the table angles and both signs of s, and every binade up to the large arguments */
	sweep("sinf", sinf, sin, 0x1p-12f, 0x1p3f, 2039, true);
	sweep("cosf", cosf, cos, 0x1p-12f, 0x1p3f, 2039, true);
	sweep("tanf", tanf, tan, 0x1p-12f, 0x1p3f, 2039, true);
	sweep("sinf", sinf, sin, 0x1p3f, 0x1p127f, 1u << 17, true);
	sweep("cosf", cosf, cos, 0x1p3f, 0x1p127f, 1u << 17, true);
	sweep("tanf", tanf, tan, 0x1p3f, 0x1p127f, 1u << 17, true);

	/* The error functions: both branches of the fixed-point core and erfc down to its subnormal results */
	sweep("erff", erff, erf, 0x1p-19f, 0x1.f5c28ep1f, 1u << 14, true);
	sweep("erfcf", erfcf, erfc, 0x1p-19f, 0x1p1f, 1u << 14, true);
	sweep("erfcf", erfcf, erfc, 0x1p1f, 0x1.28p3f, 4093, false);

	check_near();
	check_near2();

	if (failures == 0)
		puts("pass: libm_dense");

	return failures == 0 ? 0 : 1;
}
