/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * trig-wide.h
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _TRIG_WIDE_H
#define _TRIG_WIDE_H

#include <stdint.h>

/* Armv6-M multiplies 32 by 32 to 32 only: the four 16-bit products of nofp/speed's __libc_fixed_wide (D86) */
uint64_t __libc_fixed_wide(uint32_t a, uint32_t b);

static inline uint64_t __libc_trig_wide(uint32_t a, uint32_t b)
{
	return __libc_fixed_wide(a, b);
}

#endif /* _TRIG_WIDE_H */
