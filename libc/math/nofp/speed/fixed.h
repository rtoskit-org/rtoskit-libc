/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fixed.h
 *
 * Created on: Oct 21, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FIXED_H
#define _FIXED_H

#include <stdint.h>

/*
 * Fixed-point arithmetic for the speed flavour's float functions on the
 * cores without an FPU (PLAN D86, docs/research/libm-fixed.md). Armv6-M
 * multiplies 32 by 32 to 32 bits only, so the wide product is built from
 * 16-bit halves; each helper is exact.
 */

/* The 64-bit product of a and b: out of line on Armv6-M, UMULL inline on armv7-m (fixed-wide.h, picked by speed.mk) */
#include "fixed-wide.h"

/* The high 32 bits of the product of a and b (rounded down) */
static inline uint32_t __libc_fixed_high(uint32_t a, uint32_t b)
{
	return (uint32_t)(__libc_fixed_wide(a, b) >> 32);
}

/* The high 32 bits of the signed product of a and b (rounded down) */
static inline int32_t __libc_fixed_high_signed(int32_t a, int32_t b)
{
	uint32_t high = __libc_fixed_high((uint32_t)a, (uint32_t)b);

	/* The unsigned product of a two's complement operand counts the other operand 2^32 times too often */
	if (a < 0)
		high -= (uint32_t)b;
	if (b < 0)
		high -= (uint32_t)a;

	return (int32_t)high;
}

/* The 64-bit product of two signed numbers */
static inline int64_t __libc_fixed_wide_signed(int32_t a, int32_t b)
{
	uint64_t product = __libc_fixed_wide(a < 0 ? -(uint32_t)a : (uint32_t)a, b < 0 ? -(uint32_t)b : (uint32_t)b);

	return (a < 0) != (b < 0) ? -(int64_t)product : (int64_t)product;
}

#endif /* _FIXED_H */
