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

/*
 * The accuracy contract of the build type (D162, D163): bounds.c for debug, release and speed, fast/bounds.c for
 * fast, selected by subdir.mk. libm_vectors.c checks the special values, the exact functions and the exceptions
 * itself, the same in every build type, and asks these about the rest. precision_kind is 0 for double, 1 for long
 * double and 2 for float; direction 0 is to nearest.
 */
/* Whether a result to nearest whose exact value is representable (want) must be want, bit for bit */
bool libm_exact_result_required(const struct libm_function *f, int precision_kind, double want);
/*
 * Whether a finite result is within the bound: to nearest, error is in ulps of the exact value, otherwise of the
 * correctly rounded want; flags are the vector's for the direction; float_is_correctly_rounded says whether this
 * core's float functions are correctly rounded in the speed flavour (D98)
 */
bool libm_within_bound(const struct libm_function *f, int precision_kind, int direction, double got, double want, double error, int flags, bool float_is_correctly_rounded);

#endif /* _LIBM_VECTORS_H */
