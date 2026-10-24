/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * arm-sample.c
 *
 * Created on: Oct 22, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * The Arm half of the accuracy comparison (P4.05, PLAN D92): the same
 * sampled arguments evaluated on the target, where AOR's float functions run
 * their double arithmetic in soft-float and ours their float-float or
 * fixed-point paths. tools/scripts/aor-accuracy links it against
 * rtoskit-libc alone or with AOR's archives in front, runs it under the
 * architecture's runner and hands the records to the host checker
 * (tools/aor/accuracy.c, "records"), which measures them against MPFR.
 *
 * Output, per function: its name in 16 bytes, then SAMPLES records of three
 * doubles (x, y, result), in the target's byte order (little-endian, as the
 * host's).
 */

#define SAMPLES 16384

struct function {
	const char *name;
	float (*f)(float);
	double (*d)(double);
	float (*f2)(float, float);
	double (*d2)(double, double);
	double low;
	double high;
	double low2;
	double high2;
};

static const struct function functions[] = {
	{ "expf", expf, nullptr, nullptr, nullptr, -104, 89, 0, 0 },
	{ "exp2f", exp2f, nullptr, nullptr, nullptr, -150, 128, 0, 0 },
	{ "logf", logf, nullptr, nullptr, nullptr, 0, 1e30, 0, 0 },
	{ "log2f", log2f, nullptr, nullptr, nullptr, 0, 1e30, 0, 0 },
	{ "log10f", log10f, nullptr, nullptr, nullptr, 0, 1e30, 0, 0 },
	{ "sinf", sinf, nullptr, nullptr, nullptr, -1e4, 1e4, 0, 0 },
	{ "cosf", cosf, nullptr, nullptr, nullptr, -1e4, 1e4, 0, 0 },
	{ "erff", erff, nullptr, nullptr, nullptr, -5, 5, 0, 0 },
	{ "powf", nullptr, nullptr, powf, nullptr, 0, 16, -40, 40 },
	{ "exp", nullptr, exp, nullptr, nullptr, -746, 710, 0, 0 },
	{ "exp2", nullptr, exp2, nullptr, nullptr, -1075, 1024, 0, 0 },
	{ "log", nullptr, log, nullptr, nullptr, 0, 1e300, 0, 0 },
	{ "log2", nullptr, log2, nullptr, nullptr, 0, 1e300, 0, 0 },
	{ "erf", nullptr, erf, nullptr, nullptr, -6, 6, 0, 0 },
	{ "pow", nullptr, nullptr, nullptr, pow, 0, 16, -250, 250 },
};

static uint64_t next_random(uint64_t *state)
{
	*state ^= *state >> 12;
	*state ^= *state << 25;
	*state ^= *state >> 27;
	return *state * 2685821657736338717ULL;
}

/* Half the draws uniform over [low, high], half spread over the binades, as the host sampler does */
static double argument(uint64_t *state, bool is_float, double low, double high)
{
	uint64_t r = next_random(state);
	double v;

	if (r & 1) {
		v = low + (high - low) * ((double)(next_random(state) >> 11) * 0x1p-53);
	} else {
		double limit = fmax(fabs(low), fabs(high));
		int top = ilogb(limit);
		int bottom = is_float ? -149 : -1074;
		int e = bottom + (int)(next_random(state) % (uint64_t)(top - bottom + 1));

		v = ldexp(1.0 + (double)(next_random(state) >> 11) * 0x1p-53, e);
		if (low < 0 && (r & 2))
			v = -v;
		if (v < low || v > high)
			v = low + (high - low) * ((double)(next_random(state) >> 11) * 0x1p-53);
	}
	return is_float ? (float)v : v;
}

int main(void)
{
	static double records[SAMPLES][3];

	for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		const struct function *f = &functions[i];
		bool is_float = f->f != nullptr || f->f2 != nullptr;
		uint64_t state = 0x9e3779b97f4a7c15ULL + i;
		char name[16] = { 0 };

		for (size_t k = 0; k < SAMPLES; ++k) {
			double x = argument(&state, is_float, f->low, f->high);
			double y = (f->f2 || f->d2) ? argument(&state, is_float, f->low2, f->high2) : 0;
			double result;

			if (f->f)
				result = f->f((float)x);
			else if (f->d)
				result = f->d(x);
			else if (f->f2)
				result = f->f2((float)x, (float)y);
			else
				result = f->d2(x, y);
			records[k][0] = x;
			records[k][1] = y;
			records[k][2] = result;
		}
		strncpy(name, f->name, sizeof(name) - 1);
		if (fwrite(name, sizeof(name), 1, stdout) != 1 || fwrite(records, sizeof(records), 1, stdout) != 1)
			return 1;
	}
	return fflush(stdout) == 0 ? 0 : 1;
}
