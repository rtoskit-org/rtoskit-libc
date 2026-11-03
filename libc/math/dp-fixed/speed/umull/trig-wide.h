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

/* 32 x 32 -> 64 bits in one UMULL, inline: the Thumb-2 cores (armv7-m, armv7e-m, armv8-m.main; P8.03b) */
static inline uint64_t __libc_trig_wide(uint32_t a, uint32_t b)
{
	return (uint64_t)a * b;
}

#endif /* _TRIG_WIDE_H */
