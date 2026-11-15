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
 * The high 64 bits of a b on the hot Horner chains of dp-fixed/fast (P11.06d, Q53, D207), on Armv6-M: the one
 * out-of-line __libc_trig_high64 of each object, as before. Each 32 x 32 product there is four 16-bit MULS, so a
 * call is a small part of the cost and an inlined copy a large one; the build picks this header (fast.mk).
 */
static inline uint64_t __libc_dp_fixed_hot_high(uint64_t a, uint64_t b)
{
	return __libc_trig_high64(a, b);
}

#endif /* _FAST_HIGH_H */
