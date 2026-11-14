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

/*
 * 2^(j/32), j = 0..31, correctly rounded (computed with 200-bit arithmetic; the note, section 3), as bits less j 2^47
 * (P11.06b, section 18): __libc_fast_exp2 adds n 2^47 for n = 32 q + j
 */
const uint64_t __libc_fast_exp2_table[32] = {
	0x3ff0000000000000ull,
	0x3fefd9b0d3158574ull,
	0x3fefb5586cf9890full,
	0x3fef9301d0125b51ull,
	0x3fef72b83c7d517bull,
	0x3fef54873168b9aaull,
	0x3fef387a6e756238ull,
	0x3fef1e9df51fdee1ull,
	0x3fef06fe0a31b715ull,
	0x3feef1a7373aa9cbull,
	0x3feedea64c123422ull,
	0x3feece086061892dull,
	0x3feebfdad5362a27ull,
	0x3feeb42b569d4f82ull,
	0x3feeab07dd485429ull,
	0x3feea47eb03a5585ull,
	0x3feea09e667f3bcdull,
	0x3fee9f75e8ec5f74ull,
	0x3feea11473eb0187ull,
	0x3feea589994cce13ull,
	0x3feeace5422aa0dbull,
	0x3feeb737b0cdc5e5ull,
	0x3feec49182a3f090ull,
	0x3feed503b23e255dull,
	0x3feee89f995ad3adull,
	0x3feeff76f2fb5e47ull,
	0x3fef199bdd85529cull,
	0x3fef3720dcef9069ull,
	0x3fef5818dcfba487ull,
	0x3fef7c97337b9b5full,
	0x3fefa4afa2a490daull,
	0x3fefd0765b6e4540ull,
};
