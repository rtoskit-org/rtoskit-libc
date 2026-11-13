/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_sin_table.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* sin(j pi/32) for j = 0..16, each as the double nearest it and the double nearest the rest (PLAN D196) */
const struct __libc_dd __libc_fast_sin_table[17] = {
	{ 0x0.0p+0, 0x0.0p+0 },
	{ 0x1.917a6bc29b42cp-4, -0x1.e2718d26ed688p-60 },
	{ 0x1.8f8b83c69a60bp-3, -0x1.26d19b9ff8d82p-57 },
	{ 0x1.294062ed59f06p-2, -0x1.5d28da2c4612dp-56 },
	{ 0x1.87de2a6aea963p-2, -0x1.72cedd3d5a610p-57 },
	{ 0x1.e2b5d3806f63bp-2, 0x1.e0d891d3c6841p-58 },
	{ 0x1.1c73b39ae68c8p-1, 0x1.b25dd267f6600p-55 },
	{ 0x1.44cf325091dd6p-1, 0x1.8076a2cfdc6b3p-57 },
	{ 0x1.6a09e667f3bcdp-1, -0x1.bdd3413b26456p-55 },
	{ 0x1.8bc806b151741p-1, -0x1.2c5e12ed1336dp-55 },
	{ 0x1.a9b66290ea1a3p-1, 0x1.9f630e8b6dac8p-60 },
	{ 0x1.c38b2f180bdb1p-1, -0x1.6e0b1757c8d07p-56 },
	{ 0x1.d906bcf328d46p-1, 0x1.457e610231ac2p-56 },
	{ 0x1.e9f4156c62ddap-1, 0x1.760b1e2e3f81ep-55 },
	{ 0x1.f6297cff75cb0p-1, 0x1.562172a361fd3p-56 },
	{ 0x1.fd88da3d12526p-1, -0x1.87df6378811c7p-55 },
	{ 0x1.0000000000000p+0, 0x0.0p+0 },
};
