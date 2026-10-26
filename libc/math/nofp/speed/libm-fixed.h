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

#include <stdint.h>

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

#endif /* _LIBM_FIXED_H */
