/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp2_table.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/math-fast.h"

/* 2^(j/32), j = 0..31, correctly rounded (computed with 200-bit arithmetic; the note, section 3), as bits */
const uint64_t __libc_fast_exp2_table[32] = {
	0x3ff0000000000000ull,
	0x3ff059b0d3158574ull,
	0x3ff0b5586cf9890full,
	0x3ff11301d0125b51ull,
	0x3ff172b83c7d517bull,
	0x3ff1d4873168b9aaull,
	0x3ff2387a6e756238ull,
	0x3ff29e9df51fdee1ull,
	0x3ff306fe0a31b715ull,
	0x3ff371a7373aa9cbull,
	0x3ff3dea64c123422ull,
	0x3ff44e086061892dull,
	0x3ff4bfdad5362a27ull,
	0x3ff5342b569d4f82ull,
	0x3ff5ab07dd485429ull,
	0x3ff6247eb03a5585ull,
	0x3ff6a09e667f3bcdull,
	0x3ff71f75e8ec5f74ull,
	0x3ff7a11473eb0187ull,
	0x3ff82589994cce13ull,
	0x3ff8ace5422aa0dbull,
	0x3ff93737b0cdc5e5ull,
	0x3ff9c49182a3f090ull,
	0x3ffa5503b23e255dull,
	0x3ffae89f995ad3adull,
	0x3ffb7f76f2fb5e47ull,
	0x3ffc199bdd85529cull,
	0x3ffcb720dcef9069ull,
	0x3ffd5818dcfba487ull,
	0x3ffdfc97337b9b5full,
	0x3ffea4afa2a490daull,
	0x3fff50765b6e4540ull,
};
