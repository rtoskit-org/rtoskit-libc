/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_rsqrt_seed.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* 1/sqrt(t) on [(i + 8)/32, (i + 9)/32) in units of 2^-14: 2/(sqrt(a) + sqrt(b)) for the interval [a, b), 2^-5.09 (PLAN D200) */
const uint16_t __libc_dp_fixed_rsqrt_seed[24] = {
	0x7c3b,
	0x7580,
	0x6fc2,
	0x6ac9,
	0x666c,
	0x628d,
	0x5f17,
	0x5bf8,
	0x5923,
	0x568d,
	0x542e,
	0x51fe,
	0x4ff8,
	0x4e16,
	0x4c54,
	0x4ab0,
	0x4926,
	0x47b3,
	0x4655,
	0x450a,
	0x43d2,
	0x42a9,
	0x418f,
	0x4082,
};
