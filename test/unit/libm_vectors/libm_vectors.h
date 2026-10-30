/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm_vectors.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIBM_VECTORS_H
#define _LIBM_VECTORS_H

#include <stddef.h>

/* Per rounding direction: what tools/libm-vectors/libm-vectors.c writes (the values must match its enum) */
#define LIBM_INVALID 1
#define LIBM_DIVBYZERO 2
#define LIBM_OVERFLOW 4
#define LIBM_UNDERFLOW 8
#define LIBM_UNDERFLOW_ANY 16
#define LIBM_INVALID_ANY 32
#define LIBM_ZERO_SIGN_ANY 64

/*
 * One argument set: the correctly rounded result for FE_TONEAREST,
 * FE_UPWARD, FE_DOWNWARD and FE_TOWARDZERO, the exact result's offset from
 * the nearest one in its ulps, the second result of modf (extra) or the
 * integer one of frexp and remquo (aux), and the exceptions of each direction.
 */
struct libm_vector {
	double x;
	double y;
	double z;
	double result[4];
	double delta;
	double extra;
	long aux;
	unsigned char flags[4];
};

enum libm_kind {
	LIBM_UNARY,
	LIBM_BINARY,
	LIBM_TERNARY,
	LIBM_SCALE,
	LIBM_SCALBLN,
	LIBM_TO_LONG,
	LIBM_TO_LLONG,
	LIBM_ILOGB,
	LIBM_FREXP,
	LIBM_MODF,
	LIBM_REMQUO,
	LIBM_NEXTAFTER,
	LIBM_NEXTTOWARD,
};

/* A function in one precision; the pointer matches the kind */
struct libm_function {
	const char *name;
	enum libm_kind kind;
	const struct libm_vector *vectors;
	size_t count;
	int precision;
	const void *pointer;
};

extern const struct libm_function libm_double_functions[];
extern const size_t libm_double_function_count;
extern const struct libm_function libm_long_double_functions[];
extern const size_t libm_long_double_function_count;
extern const struct libm_function libm_float_functions[];
extern const size_t libm_float_function_count;

#endif /* _LIBM_VECTORS_H */
