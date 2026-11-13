/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_atan_table.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* atan(j/16) for j = 0..16 in Q64, rounded (PLAN D199) */
const uint64_t __libc_dp_fixed_atan_table[17] = {
	0x0000000000000000ull,
	0x0ffaaddb967ef4e3ull,
	0x1fd5ba9aac2f6dc6ull,
	0x2f72f6979cb6044dull,
	0x3eb6ebf25901bac5ull,
	0x4d89dcdc1faf2f35ull,
	0x5bd86507937bc23aull,
	0x6993bb0f308ff2dbull,
	0x76b19c1586ed3da3ull,
	0x832bf4a6d9867e2aull,
	0x8f005d5ef7f59f9bull,
	0x9a2f80e671bdda20ull,
	0xa4bc7d1934f70924ull,
	0xaeac4c38b4d8c080ull,
	0xb8053e2bc2319e74ull,
	0xc0ce85b8ac526641ull,
	0xc90fdaa22168c235ull,
};
