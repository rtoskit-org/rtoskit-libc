/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fast-high.h
 *
 * Created on: Nov 15, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FAST_HIGH_H
#define _FAST_HIGH_H

#include <stdint.h>

#include "trig-fixed.h"

/*
 * The high 64 bits of a b on the hot Horner chains of dp-fixed/fast (P11.06d, Q53, D207), on the Thumb-2 cores:
 * __libc_trig_high64's body, always inlined. At -Os that one stays out of line, and here a call costs about as much
 * as its two or four UMULLs; the build picks this header for the cores with UMULL (fast.mk).
 */
[[gnu::always_inline]] static inline uint64_t __libc_dp_fixed_hot_high(uint64_t a, uint64_t b)
{
	uint32_t a0 = (uint32_t)a;
	uint32_t a1 = (uint32_t)(a >> 32);
	uint32_t b0 = (uint32_t)b;
	uint32_t b1 = (uint32_t)(b >> 32);
	uint64_t low;

	/* A b of 32 bits (the early Horner terms): two products, the same value */
	if (b1 == 0)
		return (__libc_trig_wide(a1, b0) + (__libc_trig_wide(a0, b0) >> 32)) >> 32;
	low = __libc_trig_wide(a0, b0);
	uint64_t cross0 = __libc_trig_wide(a0, b1);
	uint64_t cross1 = __libc_trig_wide(a1, b0);
	uint64_t middle = (low >> 32) + (uint32_t)cross0 + (uint32_t)cross1;

	return __libc_trig_wide(a1, b1) + (cross0 >> 32) + (cross1 >> 32) + (middle >> 32);
}

#endif /* _FAST_HIGH_H */
