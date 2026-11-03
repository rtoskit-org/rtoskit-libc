/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * trig_fixed_check.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The error of the fixed-point double sine and cosine kernel (P8.03a, PLAN
 * D133, docs/research/sincos.md), measured on the host against MPFR.
 * libc/math/dp-fixed/speed/__libc_trig_fixed.c is compiled against the
 * library's headers and linked here with host stand-ins for the three
 * library functions it calls. For every argument the harness takes sin x
 * and cos x from the kernel, compares each 64-bit significand with the
 * exact value (MPFR, 256 bits) in units of its last bit, and checks that a
 * result the rounding test accepts is the correctly rounded double.
 *
 * Usage: trig_fixed_check COUNT [SEED]
 */

#include <inttypes.h>
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
bool __libc_trig_fixed_round(const struct __libc_trig_fixed *value, double *result);

/* The library functions the kernel calls */
uint64_t __libc_fixed_wide(uint32_t a, uint32_t b)
{
	return (uint64_t)a * b;
}

unsigned int stdc_leading_zeros_ui(unsigned int x)
{
	return x == 0 ? 32 : (unsigned int)__builtin_clz(x);
}

unsigned int stdc_leading_zeros_ull(unsigned long long x)
{
	return x == 0 ? 64 : (unsigned int)__builtin_clzll(x);
}

struct tally {
	double worst;
	double worst_x;
	unsigned long long arguments;
	unsigned long long declined;
	unsigned long long fallback;
	unsigned long long wrong;
};

static struct tally tallies[2];
static mpfr_t exact;
static mpfr_t value;
static mpfr_t difference;

/* |kernel - exact| in units of the kernel significand's last bit */
static double error_units(const struct __libc_trig_fixed *k, mpfr_t truth)
{
	mpfr_set_ui_2exp(value, 0, 0, MPFR_RNDN);
	mpfr_set_uj(value, k->v, MPFR_RNDN);
	mpfr_mul_2si(value, value, k->exponent - 63, MPFR_RNDN);
	if (k->negative)
		mpfr_neg(value, value, MPFR_RNDN);
	mpfr_sub(difference, value, truth, MPFR_RNDN);
	mpfr_abs(difference, difference, MPFR_RNDN);
	mpfr_mul_2si(difference, difference, 63 - k->exponent, MPFR_RNDN);
	return mpfr_get_d(difference, MPFR_RNDU);
}

static void check(double x)
{
	struct __libc_trig_fixed results[2];

	if (!__libc_trig_fixed(x, 3, &results[0], &results[1])) {
		tallies[0].declined++;
		tallies[1].declined++;
		return;
	}
	for (int f = 0; f < 2; ++f) {
		struct tally *t = &tallies[f];
		double rounded;
		double units;

		t->arguments++;
		mpfr_set_d(exact, x, MPFR_RNDN);
		if (f == 0)
			mpfr_sin(exact, exact, MPFR_RNDN);
		else
			mpfr_cos(exact, exact, MPFR_RNDN);
		units = error_units(&results[f], exact);
		if (units > t->worst) {
			t->worst = units;
			t->worst_x = x;
		}
		if (!__libc_trig_fixed_round(&results[f], &rounded)) {
			t->fallback++;
			continue;
		}
		if (rounded != mpfr_get_d(exact, MPFR_RNDN)) {
			t->wrong++;
			printf("wrong: %s(%a) = %a, want %a\n", f == 0 ? "sin" : "cos", x, rounded, mpfr_get_d(exact, MPFR_RNDN));
		}
	}
}

static uint64_t state;

static uint64_t next(void)
{
	/* splitmix64 */
	uint64_t z = (state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int main(int argc, char **argv)
{
	unsigned long long count = argc > 1 ? strtoull(argv[1], NULL, 0) : 1000000;

	state = argc > 2 ? strtoull(argv[2], NULL, 0) : 1;
	mpfr_inits2(256, exact, value, difference, (mpfr_ptr)0);

	/* The doubles nearest n pi/2 and their neighbours, where the reduction cancels most */
	mpfr_t pio2;
	mpfr_init2(pio2, 256);
	mpfr_const_pi(pio2, MPFR_RNDN);
	mpfr_div_2ui(pio2, pio2, 1, MPFR_RNDN);
	for (long n = 1; n < 667544; n += (n < 4096 ? 1 : 97)) {
		mpfr_t t;
		double d;

		mpfr_init2(t, 256);
		mpfr_mul_si(t, pio2, n, MPFR_RNDN);
		d = mpfr_get_d(t, MPFR_RNDN);
		mpfr_clear(t);
		for (int k = -2; k <= 2; ++k) {
			double y = d;

			for (int i = 0; i < (k < 0 ? -k : k); ++i)
				y = nextafter(y, k < 0 ? 0.0 : INFINITY);
			check(y);
			check(-y);
		}
	}

	/* Random: half log-uniform over [2^-27, 2^20), half uniform over [-10, 10] */
	for (unsigned long long i = 0; i < count; ++i) {
		uint64_t r = next();
		double x;

		if (i & 1) {
			x = ldexp(1.0 + (double)(r >> 12) * 0x1p-52, (int)((r & 0x3f) % 47) - 27);
			if (r & 0x40)
				x = -x;
		} else {
			x = ((double)(r >> 11) * 0x1p-53) * 20.0 - 10.0;
		}
		if (fabs(x) < 0x1p-27)
			continue;
		check(x);
	}

	int status = 0;

	for (int f = 0; f < 2; ++f) {
		struct tally *t = &tallies[f];

		printf("%s: %llu arguments, largest error %.3f units at %a, %llu declined, %llu fall back (%.3f %%), %llu wrong\n", f == 0 ? "sin" : "cos", t->arguments, t->worst, t->worst_x, t->declined, t->fallback, 100.0 * (double)t->fallback / (double)(t->arguments ? t->arguments : 1), t->wrong);
		if (t->wrong)
			status = 1;
	}
	return status;
}
