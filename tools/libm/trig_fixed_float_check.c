/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * trig_fixed_float_check.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Every float argument of the D136 kernel's float path (P9.02, Q41, PLAN
 * D141): sinf and cosf of every positive float in [2^-27, 2^20) through
 * __libc_trig_fixed and __libc_trig_fixed_roundf (sin is odd and cos even,
 * and the kernel takes the sign apart, so the negative floats round alike).
 * A float result whose 40 discarded bits lie more than 2^30 units from the
 * halfway point cannot be misrounded by a kernel error below 2^30 units
 * (measured: 7.08); every other result the rounding test accepts is
 * checked against MPFR at 256 bits, with the kernel's error in units.
 *
 * Usage: trig_fixed_float_check PART PARTS   the PART-th of PARTS slices
 */

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/* After <stdint.h>, for mpfr_set_uj */
#include <mpfr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct __libc_trig_fixed {
	uint64_t v;
	int exponent;
	bool negative;
};

bool __libc_trig_fixed(double x, unsigned int want, struct __libc_trig_fixed *sine, struct __libc_trig_fixed *cosine);
bool __libc_trig_fixed_roundf(const struct __libc_trig_fixed *value, float *result);

int main(int argc, char **argv)
{
	uint32_t part = argc > 1 ? (uint32_t)atoi(argv[1]) : 0;
	uint32_t parts = argc > 2 ? (uint32_t)atoi(argv[2]) : 1;
	const uint32_t low = 0x32000000u;
	const uint32_t high = 0x49800000u;
	unsigned long long arguments = 0;
	unsigned long long declined[2] = { 0, 0 };
	unsigned long long checked[2] = { 0, 0 };
	unsigned long long wrong[2] = { 0, 0 };
	double worst[2] = { 0, 0 };
	mpfr_t exact;
	mpfr_t value;

	mpfr_inits2(256, exact, value, (mpfr_ptr)0);
	for (uint32_t bits = low + part; bits < high; bits += parts) {
		float xf;
		struct __libc_trig_fixed k[2];

		memcpy(&xf, &bits, sizeof(xf));
		++arguments;
		if (!__libc_trig_fixed((double)xf, 3, &k[0], &k[1])) {
			declined[0]++;
			declined[1]++;
			continue;
		}
		for (int f = 0; f < 2; ++f) {
			float rounded;
			uint64_t rest = k[f].v & 0x000000ffffffffffULL;
			uint64_t distance = rest > 0x0000008000000000ULL ? rest - 0x0000008000000000ULL : 0x0000008000000000ULL - rest;

			if (!__libc_trig_fixed_roundf(&k[f], &rounded)) {
				declined[f]++;
				continue;
			}
			if (distance > (1ULL << 30))
				continue;
			checked[f]++;
			mpfr_set_flt(exact, xf, MPFR_RNDN);
			if (f == 0)
				mpfr_sin(exact, exact, MPFR_RNDN);
			else
				mpfr_cos(exact, exact, MPFR_RNDN);
			mpfr_set_uj(value, k[f].v, MPFR_RNDN);
			mpfr_mul_2si(value, value, k[f].exponent - 63, MPFR_RNDN);
			if (k[f].negative)
				mpfr_neg(value, value, MPFR_RNDN);
			mpfr_sub(value, value, exact, MPFR_RNDN);
			mpfr_abs(value, value, MPFR_RNDN);
			mpfr_mul_2si(value, value, 63 - k[f].exponent, MPFR_RNDN);
			if (mpfr_get_d(value, MPFR_RNDU) > worst[f])
				worst[f] = mpfr_get_d(value, MPFR_RNDU);
			if (rounded != mpfr_get_flt(exact, MPFR_RNDN)) {
				wrong[f]++;
				printf("wrong: %s(%a) = %a, want %a\n", f == 0 ? "sinf" : "cosf", (double)xf, (double)rounded, (double)mpfr_get_flt(exact, MPFR_RNDN));
			}
		}
	}
	for (int f = 0; f < 2; ++f)
		printf("%s: %llu arguments, %llu declined, %llu near halfway checked with MPFR, largest error there %.3f units, %llu wrong\n", f == 0 ? "sinf" : "cosf", arguments, declined[f], checked[f], worst[f], wrong[f]);
	return wrong[0] || wrong[1];
}
