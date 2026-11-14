/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_logf_log10.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* -log(1/c_i) in base 10, as a multiple of 2^-16 and the rest, both computed with 200-bit arithmetic (the note, section 8) */
const float __libc_fast_logf_log10[32][2] = {
	{ -0x1.3458000000000p-3f, -0x1.8a00ac0000000p-19f },
	{ -0x1.20e8000000000p-3f, -0x1.8ea6380000000p-21f },
	{ -0x1.0de0000000000p-3f, -0x1.b39fac0000000p-19f },
	{ -0x1.f680000000000p-4f, -0x1.09edac0000000p-19f },
	{ -0x1.d200000000000p-4f, -0x1.19ca4e0000000p-18f },
	{ -0x1.ae40000000000p-4f, -0x1.414cde0000000p-19f },
	{ -0x1.8b30000000000p-4f, -0x1.40e0240000000p-18f },
	{ -0x1.68d0000000000p-4f, -0x1.39aa220000000p-18f },
	{ -0x1.4720000000000p-4f, 0x1.156a540000000p-18f },
	{ -0x1.2600000000000p-4f, -0x1.83872e0000000p-19f },
	{ -0x1.0580000000000p-4f, -0x1.4b50f60000000p-18f },
	{ -0x1.cb40000000000p-5f, 0x1.bf63480000000p-19f },
	{ -0x1.8c80000000000p-5f, -0x1.e4a7020000000p-19f },
	{ -0x1.4ee0000000000p-5f, -0x1.81b6a60000000p-18f },
	{ -0x1.1260000000000p-5f, 0x1.838be00000000p-20f },
	{ -0x1.adc0000000000p-6f, 0x1.cfe9280000000p-18f },
	{ -0x1.3880000000000p-6f, -0x1.9386420000000p-22f },
	{ -0x1.8a80000000000p-7f, -0x1.7c7f980000000p-20f },
	{ -0x1.5000000000000p-8f, 0x1.f6c0a80000000p-18f },
	{ 0x0p+0f, 0x0p+0f },
	{ 0x1.10c0000000000p-6f, -0x1.7d135a0000000p-18f },
	{ 0x1.e380000000000p-6f, 0x1.b9f35a0000000p-24f },
	{ 0x1.5820000000000p-5f, 0x1.cb26a00000000p-20f },
	{ 0x1.bba0000000000p-5f, 0x1.3426e80000000p-18f },
	{ 0x1.0e40000000000p-4f, -0x1.6b59740000000p-19f },
	{ 0x1.3d60000000000p-4f, -0x1.66ff8e0000000p-19f },
	{ 0x1.6b40000000000p-4f, 0x1.776ca20000000p-18f },
	{ 0x1.9800000000000p-4f, 0x1.b5c55a0000000p-18f },
	{ 0x1.c3b0000000000p-4f, -0x1.57a9320000000p-20f },
	{ 0x1.ee50000000000p-4f, -0x1.51c9040000000p-18f },
	{ 0x1.0bf0000000000p-3f, 0x1.e8f0840000000p-18f },
	{ 0x1.2048000000000p-3f, 0x1.03863a0000000p-20f },
};
