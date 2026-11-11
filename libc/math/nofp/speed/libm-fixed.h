/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * libm-fixed.h
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIBM_FIXED_H
#define _LIBM_FIXED_H

#include <stdbit.h>
#include <stdbool.h>
#include <stdint.h>

#include "fixed.h"

/*
 * The speed flavour's fixed-point float functions on the cores without an
 * FPU (PLAN D86, docs/research/libm-fixed.md): a significand in [1, 2) as
 * 1 + F 2^-32, rounded at once unless F lies within its error bound of the
 * point halfway between two floats; the caller then takes the float-float
 * path (D57), so every result is the one that path gives or closer.
 */

/*
 * Bounds, in units of 2^-32, on the error of F, above the largest error
 * measured over every float argument (2.71 for the exponentials, 4.49 for
 * the logarithms) with margin
 */
#define __LIBC_FIXED_EXP2_ERROR 6u
#define __LIBC_FIXED_LOG2_ERROR 8u
#define __LIBC_FIXED_SIN_ERROR 3u
#define __LIBC_FIXED_TAN_ERROR 4u
#define __LIBC_FIXED_ERF_ERROR 8u

/* The log2 reduction tables shared by the logarithm cores (__libc_log2f_table.c) */
extern const uint32_t __libc_log2f_inverse[23];
extern const int64_t __libc_log2f_logarithm[23];

uint32_t __libc_exp2f_fixed(int64_t z, int *exponent);
uint64_t __libc_log2f_fixed(uint32_t bits, int *exponent, bool *negative);
uint64_t __libc_log2f_wide(uint32_t bits, int *exponent, bool *negative);
uint64_t __libc_sinf_fixed(int k, float high, float low, int *exponent, bool *negative);
uint64_t __libc_erff_fixed(uint32_t bits, bool complement, int *exponent);

/*
 * The float 2^exponent (1 + f 2^-32) rounded to nearest, for an exponent of
 * the normal range, in *bits; false when f is within error of a halfway
 * point (the 2^-33 bit alone set below the float's last bit), where the
 * rounding is not certain. A carry out of the significand reaches the
 * exponent through the addition.
 */
static inline bool __libc_fixed_round(uint32_t f, int exponent, uint32_t error, uint32_t *bits)
{
	uint32_t below = f & 0x1ff;
	uint32_t distance = below > 0x100 ? below - 0x100 : 0x100 - below;

	*bits = ((uint32_t)(exponent + 127) << 23) + (f >> 9) + (below > 0x100);
	return distance > error;
}

/*
 * The float 2^exponent (1 + f 2^-32) rounded to nearest without the test (the fast flavour, libc/math/nofp/fast/,
 * PLAN D172): the result is within 0.5 + error/512 ulp
 */
static inline uint32_t __libc_fixed_nearest(uint32_t f, int exponent)
{
	return ((uint32_t)(exponent + 127) << 23) + (f >> 9) + ((f & 0x1ff) > 0x100);
}

/*
 * The log2 core of __libc_log2f_fixed (libc/math/nofp/speed/__libc_log2f_fixed.c, whose comment it follows and whose
 * arithmetic it repeats step for step) from 2^e m with m in [sqrt(1/2), sqrt(2)) as an unsigned Q31 number, for the fast
 * flavour's log1pf family (PLAN D174), which feeds it 1 + x exactly with up to 30 significant bits: r = m / c' - 1 is still
 * an exact Q62 product less 1. The speed file keeps its own copy, which inlining this one would reorder (one
 * instruction more on armv7-m, against the locked speed figures)
 */
static inline uint64_t __libc_log2_fixed_core(uint32_t m, int e, int *exponent, bool *negative)
{
	uint64_t magnitude;
	uint64_t product;
	int64_t r;
	int32_t r36;
	int32_t h;
	uint32_t q;
	int scale;
	int shift;
	int j;

	/* j = round(32 (m - 1)), in -9 .. 13 */
	j = ((int32_t)(m - 0x80000000u) + (1 << 25)) >> 26;
	r = (int64_t)(__libc_fixed_wide(m, __libc_log2f_inverse[j + 9]) - (UINT64_C(1) << 62));
	if (r == 0 && e == 0 && j == 0)
		return 0;

	r36 = (int32_t)((r + (1 << 25)) >> 26);
	h = -516360668;
	h = 619632802 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -774541002 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = 1032721336 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	h = -1549082005 + ((__libc_fixed_high_signed(r36, h) + 8) >> 4);
	q = 0xb8aa3b29u + (uint32_t)((__libc_fixed_high_signed(r36, h) + 8) >> 4);

	/* |r| Q = product 2^(-61 - shift) */
	magnitude = r < 0 ? -(uint64_t)r : (uint64_t)r;
	shift = magnitude != 0 ? (int)stdc_leading_zeros(magnitude) : 0;
	product = __libc_fixed_wide((uint32_t)((magnitude << shift) >> 32), q);

	if (e == 0 && j == 0) {
		*negative = r < 0;
		scale = -61 - shift;
	} else {
		int64_t sum = ((int64_t)e << 55) + __libc_log2f_logarithm[j + 9];
		int64_t term = 6 + shift < 64 ? (int64_t)(product >> (6 + shift)) : 0;

		sum += r < 0 ? -term : term;
		*negative = sum < 0;
		product = sum < 0 ? -(uint64_t)sum : (uint64_t)sum;
		scale = -55;
	}

	shift = (int)stdc_leading_zeros(product);
	*exponent = 63 + scale - shift;
	return product << shift;
}

#endif /* _LIBM_FIXED_H */
