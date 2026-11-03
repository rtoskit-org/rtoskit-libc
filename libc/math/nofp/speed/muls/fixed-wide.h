/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fixed-wide.h
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FIXED_WIDE_H
#define _FIXED_WIDE_H

#include <stdint.h>

/* The 64-bit product of a and b: Armv6-M multiplies 32 by 32 to 32, so four 16-bit products, shared out of line (__libc_fixed_wide.c, D86) */
uint64_t __libc_fixed_wide(uint32_t a, uint32_t b);

#endif /* _FIXED_WIDE_H */
