/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * trig-fixed.h
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _TRIG_FIXED_H
#define _TRIG_FIXED_H

#include <stdint.h>

/*
 * Fixed-point double sine and cosine for the cores without a double FPU
 * (P8.03, PLAN D133, docs/research/sincos.md). A result is a 64-bit
 * significand v with bit 63 set, an exponent and a sign: the value is
 * (-1)^negative v 2^(exponent - 63). Its error is below
 * __LIBC_TRIG_FIXED_ERROR units of v's last bit (measured by
 * tools/scripts/trig-fixed-check).
 */
struct __libc_trig_fixed {
	uint64_t v;
	int exponent;
	bool negative;
};

#define __LIBC_TRIG_FIXED_ERROR 18u

/* What __libc_trig_fixed computes */
#define __LIBC_TRIG_FIXED_SIN 1u
#define __LIBC_TRIG_FIXED_COS 2u

/*
 * 32 x 32 -> 64 bits, __libc_trig_wide: UMULL inline on the Thumb-2 cores,
 * four 16-bit products on Armv6-M; the build picks the header (speed.mk)
 */
#include "trig-wide.h"

/*
 * sin x and cos x (as want asks) for 2^-27 <= |x| < 2^20; false when the
 * argument needs the double-double path (|x| >= 2^20, or a reduction that
 * cancels more than 64 bits)
 */
bool __libc_trig_fixed(double x, unsigned int want, struct __libc_trig_fixed *sine, struct __libc_trig_fixed *cosine);

/*
 * The double nearest a fixed-point result, or false when the result lies
 * within the error bound of a halfway point and the double-double path must
 * decide. The soft-double cores round to nearest only (D44).
 */
bool __libc_trig_fixed_round(const struct __libc_trig_fixed *value, double *result);

/*
 * The double-double path of sin and cos (sin.c, cos.c) for an argument past
 * their entry tests: the results want asks, as those functions return them
 */
void __libc_trig_dd(double x, unsigned int want, double *sine, double *cosine);

/*
 * |x|'s bits for the entry tests, which the speed files make on the bits
 * rather than with soft-double compares: NaN and infinity at and above
 * 0x7ff0..., the tiny arguments of sin.c and cos.c below 2^-27
 */
#define __LIBC_TRIG_FIXED_INF 0x7ff0000000000000ULL
#define __LIBC_TRIG_FIXED_TINY 0x3e40000000000000ULL

static inline uint64_t __libc_trig_fixed_abs_bits(double x)
{
	uint64_t bits;

	__builtin_memcpy(&bits, &x, sizeof(bits));
	return bits & 0x7fffffffffffffffULL;
}

#endif /* _TRIG_FIXED_H */
