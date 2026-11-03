/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * trig_fixed_near.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Hard cases for the fixed-point double sine and cosine (P8.03b, PLAN D133,
 * docs/research/sincos.md): arguments whose 64-bit kernel result lies
 * within the error bound of a halfway point, so the kernel must decline,
 * where rounding the kernel result anyway gives the wrong double, and where
 * the double-double path (the library's x86_64 release sin, cos and tan, prefixed
 * rt_) is correctly rounded. A kernel that rounded without its test fails
 * on every one; both flavours must return the correct double.
 * Prints C initializers for test/unit/libm_dense/near_double.inc.
 *
 * Usage: trig_fixed_near COUNT [SEED]   COUNT cases per function
 */

#include <math.h>
#include <mpfr.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct __libc_trig_fixed {
	uint64_t v;
	int exponent;
	bool negative;
};

bool __libc_trig_fixed(double x, unsigned int want, struct __libc_trig_fixed *sine, struct __libc_trig_fixed *cosine);
bool __libc_tan_fixed(double x, struct __libc_trig_fixed *tangent);
double rt_sin(double x);
double rt_cos(double x);
double rt_tan(double x);

/* The kernel result rounded to nearest without the error test */
static double round_blindly(const struct __libc_trig_fixed *k)
{
	uint64_t significand = (k->v >> 11) + ((k->v & 0x7ff) > 0x400);
	int exponent = k->exponent;
	uint64_t bits;
	double d;

	if (significand >> 53) {
		significand >>= 1;
		exponent += 1;
	}
	bits = ((uint64_t)k->negative << 63) | ((uint64_t)(exponent + 1023) << 52) | (significand & 0x000fffffffffffffULL);
	memcpy(&d, &bits, sizeof(d));
	return d;
}

static uint64_t state;

static uint64_t next(void)
{
	uint64_t z = (state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int main(int argc, char **argv)
{
	int count = argc > 1 ? atoi(argv[1]) : 16;
	int found[3] = { 0, 0, 0 };
	mpfr_t exact;

	state = argc > 2 ? strtoull(argv[2], NULL, 0) : 7;
	mpfr_init2(exact, 256);
	while (found[0] < count || found[1] < count || found[2] < count) {
		uint64_t r = next();
		double x = (r & 1) ? ldexp(1.0 + (double)(r >> 12) * 0x1p-52, (int)((r >> 1) % 47) - 27) : ((double)(r >> 11) * 0x1p-53) * 20.0 - 10.0;
		struct __libc_trig_fixed k[3];

		if (fabs(x) < 0x1p-27 || !__libc_trig_fixed(x, 3, &k[0], &k[1]) || !__libc_tan_fixed(x, &k[2]))
			continue;
		for (int f = 0; f < 3; ++f) {
			uint32_t rest = (uint32_t)k[f].v & 0x7ff;
			double want;
			double dd;

			/* Within 4 units of halfway: inside every bound the kernel could use */
			if (found[f] >= count || rest - (0x400 - 4) > 8)
				continue;
			mpfr_set_d(exact, x, MPFR_RNDN);
			if (f == 0)
				mpfr_sin(exact, exact, MPFR_RNDN);
			else if (f == 1)
				mpfr_cos(exact, exact, MPFR_RNDN);
			else
				mpfr_tan(exact, exact, MPFR_RNDN);
			want = mpfr_get_d(exact, MPFR_RNDN);
			dd = f == 0 ? rt_sin(x) : f == 1 ? rt_cos(x) :
											   rt_tan(x);
			if (round_blindly(&k[f]) == want || dd != want)
				continue;
			printf("\t{ %s, %a, %a },\n", f == 0 ? "sin" : f == 1 ? "cos" :
																	"tan",
				   x, want);
			++found[f];
		}
	}
	return 0;
}
