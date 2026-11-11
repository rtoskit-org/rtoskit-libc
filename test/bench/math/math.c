/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * math.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* sincos and sincosf (PLAN D132) */
#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>

#include "bench.h"

/*
 * libm per function, float and double (P3.01, docs/research/benchmarks.md):
 * 16 arguments spread over the domain a program typically uses, each case
 * timed two ways. Throughput sums independent calls; latency feeds every
 * result back into the next argument (x + y * 0, which the compiler may not
 * fold without -ffast-math), as glibc's benchtests do. Under the instruction
 * counting of qemu the two agree; with a cycle counter the difference shows
 * how much the calls overlap. Operations are calls.
 */

#define ARGUMENTS 16

struct math_function {
	const char *name;
	float (*unary_float)(float);
	double (*unary_double)(double);
	float (*binary_float)(float, float);
	double (*binary_double)(double, double);
	double low;
	double high;
	double low2;
	double high2;
};

struct math_case {
	const struct math_function *function;
	float floats[ARGUMENTS];
	float floats2[ARGUMENTS];
	double doubles[ARGUMENTS];
	double doubles2[ARGUMENTS];
};

#define UNARY(name, lo, hi)                                  \
	{                                                        \
		#name, name##f, name, nullptr, nullptr, lo, hi, 0, 0 \
	}
#define BINARY(name, lo, hi, lo2, hi2)                           \
	{                                                            \
		#name, nullptr, nullptr, name##f, name, lo, hi, lo2, hi2 \
	}

/* sincos and sincosf as unary cases: the sum of the two results (P8.01) */
static double sincos_sum(double x)
{
	double s;
	double c;

	sincos(x, &s, &c);
	return s + c;
}

static float sincosf_sum(float x)
{
	float s;
	float c;

	sincosf(x, &s, &c);
	return s + c;
}

static const struct math_function functions[] = {
	UNARY(sqrt, 0, 1e4),
	UNARY(cbrt, -1e3, 1e3),
	BINARY(hypot, -1e3, 1e3, -1e3, 1e3),
	UNARY(exp, -10, 10),
	UNARY(exp2, -20, 20),
	UNARY(exp10, -10, 10),
	UNARY(expm1, -1, 1),
	UNARY(exp2m1, -1, 1),
	UNARY(exp10m1, -0.5, 0.5),
	UNARY(log, 1e-3, 1e3),
	UNARY(log2, 1e-3, 1e3),
	UNARY(log10, 1e-3, 1e3),
	UNARY(log1p, -0.5, 10),
	UNARY(log2p1, -0.5, 10),
	UNARY(log10p1, -0.5, 10),
	BINARY(pow, 0.5, 4, -8, 8),
	UNARY(sin, -10, 10),
	UNARY(cos, -10, 10),
	{ "sincos", sincosf_sum, sincos_sum, nullptr, nullptr, -10, 10, 0, 0 },
	UNARY(tan, -1.5, 1.5),
	UNARY(sinpi, -10, 10),
	UNARY(cospi, -10, 10),
	UNARY(tanpi, -0.45, 0.45),
	UNARY(asin, -1, 1),
	UNARY(acos, -1, 1),
	UNARY(atan, -10, 10),
	BINARY(atan2, -10, 10, -10, 10),
	UNARY(sinh, -10, 10),
	UNARY(cosh, -10, 10),
	UNARY(tanh, -5, 5),
	UNARY(asinh, -10, 10),
	UNARY(acosh, 1, 100),
	UNARY(atanh, -0.9, 0.9),
	UNARY(erf, -3, 3),
	UNARY(erfc, -3, 8),
	UNARY(lgamma, 0.1, 30),
	UNARY(tgamma, 0.5, 20),
	BINARY(fmod, -1e3, 1e3, 0.5, 10),
	UNARY(floor, -1e3, 1e3),
	UNARY(round, -1e3, 1e3),
};

static struct math_case cases[sizeof(functions) / sizeof(functions[0])];

static void run_float_throughput(void *context, unsigned long iterations)
{
	const struct math_case *c = context;

	for (unsigned long i = 0; i < iterations; ++i) {
		float sum = 0;

		bench_clobber(c);
		if (c->function->unary_float != nullptr) {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				sum += c->function->unary_float(c->floats[k]);
		} else {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				sum += c->function->binary_float(c->floats[k], c->floats2[k]);
		}
		bench_sink = (unsigned long)(long)sum;
	}
}

static void run_float_latency(void *context, unsigned long iterations)
{
	const struct math_case *c = context;

	for (unsigned long i = 0; i < iterations; ++i) {
		float y = 0;

		bench_clobber(c);
		if (c->function->unary_float != nullptr) {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				y = c->function->unary_float(c->floats[k] + y * 0.0f);
		} else {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				y = c->function->binary_float(c->floats[k] + y * 0.0f, c->floats2[k]);
		}
		bench_sink = (unsigned long)(long)y;
	}
}

static void run_double_throughput(void *context, unsigned long iterations)
{
	const struct math_case *c = context;

	for (unsigned long i = 0; i < iterations; ++i) {
		double sum = 0;

		bench_clobber(c);
		if (c->function->unary_double != nullptr) {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				sum += c->function->unary_double(c->doubles[k]);
		} else {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				sum += c->function->binary_double(c->doubles[k], c->doubles2[k]);
		}
		bench_sink = (unsigned long)(long long)sum;
	}
}

static void run_double_latency(void *context, unsigned long iterations)
{
	const struct math_case *c = context;

	for (unsigned long i = 0; i < iterations; ++i) {
		double y = 0;

		bench_clobber(c);
		if (c->function->unary_double != nullptr) {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				y = c->function->unary_double(c->doubles[k] + y * 0.0);
		} else {
			for (size_t k = 0; k < ARGUMENTS; ++k)
				y = c->function->binary_double(c->doubles[k] + y * 0.0, c->doubles2[k]);
		}
		bench_sink = (unsigned long)(long long)y;
	}
}

/* Arguments inside the cells of an even grid; the second argument visits the cells in another order */
static void prepare(struct math_case *c, const struct math_function *function)
{
	c->function = function;
	for (size_t k = 0; k < ARGUMENTS; ++k) {
		double x = function->low + (function->high - function->low) * ((double)k + 0.37) / ARGUMENTS;
		double y = function->low2 + (function->high2 - function->low2) * ((double)(k * 7 % ARGUMENTS) + 0.61) / ARGUMENTS;

		c->doubles[k] = x;
		c->doubles2[k] = y;
		c->floats[k] = (float)x;
		c->floats2[k] = (float)y;
	}
}

int main(int argc, char *argv[])
{
	char name[48];

	bench_init(argc, argv);

	for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		prepare(&cases[i], &functions[i]);
		snprintf(name, sizeof(name), "%sf/throughput", functions[i].name);
		bench_run(name, ARGUMENTS, run_float_throughput, &cases[i]);
		snprintf(name, sizeof(name), "%sf/latency", functions[i].name);
		bench_run(name, ARGUMENTS, run_float_latency, &cases[i]);
	}
	for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
		snprintf(name, sizeof(name), "%s/throughput", functions[i].name);
		bench_run(name, ARGUMENTS, run_double_throughput, &cases[i]);
		snprintf(name, sizeof(name), "%s/latency", functions[i].name);
		bench_run(name, ARGUMENTS, run_double_latency, &cases[i]);
	}

	return bench_finish();
}
