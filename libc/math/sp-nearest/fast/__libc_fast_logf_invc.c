/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_logf_invc.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* 1/c_i rounded to float, c_i the middle of entry i of the reduction, c = 1 for the entry holding 1 (the note, section 8) */
const float __libc_fast_logf_invc[32] = {
	0x1.6a13ce0000000p+0f,
	0x1.623fa80000000p+0f,
	0x1.5ac0560000000p+0f,
	0x1.5390940000000p+0f,
	0x1.4cab880000000p+0f,
	0x1.460cbc0000000p+0f,
	0x1.3fb0140000000p+0f,
	0x1.3991c20000000p+0f,
	0x1.33ae460000000p+0f,
	0x1.2e025c0000000p+0f,
	0x1.288b020000000p+0f,
	0x1.2345680000000p+0f,
	0x1.1e2ef40000000p+0f,
	0x1.1945380000000p+0f,
	0x1.1485f00000000p+0f,
	0x1.0fef020000000p+0f,
	0x1.0b7e6e0000000p+0f,
	0x1.0732600000000p+0f,
	0x1.03091c0000000p+0f,
	0x1.0000000000000p+0f,
	0x1.ecc07c0000000p-1f,
	0x1.de5d6e0000000p-1f,
	0x1.d0cb580000000p-1f,
	0x1.c3f8f00000000p-1f,
	0x1.b7d6c40000000p-1f,
	0x1.ac57020000000p-1f,
	0x1.a16d400000000p-1f,
	0x1.970e500000000p-1f,
	0x1.8d30180000000p-1f,
	0x1.83c9780000000p-1f,
	0x1.7ad2200000000p-1f,
	0x1.7242880000000p-1f,
};
