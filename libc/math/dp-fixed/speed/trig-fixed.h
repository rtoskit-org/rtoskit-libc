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

/* tan's: sin x / cos x by a 64-bit reciprocal (P9.03, measured by tools/scripts/trig-fixed-check) */
#define __LIBC_TAN_FIXED_ERROR 56u

/* What __libc_trig_fixed computes */
#define __LIBC_TRIG_FIXED_SIN 1u
#define __LIBC_TRIG_FIXED_COS 2u

/*
 * 32 x 32 -> 64 bits, __libc_trig_wide: UMULL inline on the Thumb-2 cores,
 * four 16-bit products on Armv6-M; the build picks the header (speed.mk)
 */
#include "trig-wide.h"

/* The high 64 bits of a b, exactly */
static inline uint64_t __libc_trig_high64(uint64_t a, uint64_t b)
{
	uint32_t a0 = (uint32_t)a;
	uint32_t a1 = (uint32_t)(a >> 32);
	uint32_t b0 = (uint32_t)b;
	uint32_t b1 = (uint32_t)(b >> 32);
	uint64_t low;

	/* Horner's tail coefficients fit in 32 bits: two products */
	if (b1 == 0)
		return (__libc_trig_wide(a1, b0) + (__libc_trig_wide(a0, b0) >> 32)) >> 32;

	low = __libc_trig_wide(a0, b0);
	uint64_t cross0 = __libc_trig_wide(a0, b1);
	uint64_t cross1 = __libc_trig_wide(a1, b0);
	uint64_t middle = (low >> 32) + (uint32_t)cross0 + (uint32_t)cross1;

	return __libc_trig_wide(a1, b1) + (cross0 >> 32) + (cross1 >> 32) + (middle >> 32);
}

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
bool __libc_trig_fixed_round(const struct __libc_trig_fixed *value, unsigned int error, double *result);

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

/*
 * The float nearest a fixed-point result, or false when the result lies
 * within the error bound of a float halfway point, or would be subnormal
 * (P9.02, Q41): the 40 bits below the float significand decide
 */
bool __libc_trig_fixed_roundf(const struct __libc_trig_fixed *value, unsigned int error, float *result);

/*
 * A float argument for the kernel: true, with x as a double, when |x| is in
 * the kernel's domain, [2^-27, 2^20); built from the bits, so Armv6-M calls
 * no soft-float conversion
 */
static inline bool __libc_trig_fixed_from_float(float x, double *d)
{
	uint32_t bits;
	uint32_t magnitude;
	uint64_t wide;

	__builtin_memcpy(&bits, &x, sizeof(bits));
	magnitude = bits & 0x7fffffffu;
	if (magnitude < 0x32000000u || magnitude >= 0x49800000u)
		return false;

	/* A normal float: rebias the exponent (127 -> 1023) and widen the significand */
	wide = ((uint64_t)(bits >> 31) << 63) | ((uint64_t)((magnitude >> 23) + 1023 - 127) << 52) | ((uint64_t)(magnitude & 0x007fffffu) << 29);
	__builtin_memcpy(d, &wide, sizeof(*d));
	return true;
}

/*
 * tan x for 2^-27 <= |x| < 2^20 as sin x / cos x from one kernel call, by a
 * 64-bit reciprocal of the cosine's significand (P9.03, Q42); false when the
 * kernel declines
 */
bool __libc_tan_fixed(double x, struct __libc_trig_fixed *tangent);

/* tan.c's double-double path for an argument past its entry tests */
double __libc_tan_dd(double x);

#endif /* _TRIG_FIXED_H */
