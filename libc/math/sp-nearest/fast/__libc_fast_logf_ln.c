/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_logf_ln.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* -log(1/c_i) in natural, as a multiple of 2^-16 and the rest, both computed with 200-bit arithmetic (the note, section 8) */
const float __libc_fast_logf_ln[32][2] = {
	{ -0x1.6300000000000p-2f, -0x1.9a5d560000000p-21f },
	{ -0x1.4ca0000000000p-2f, 0x1.f47e8e0000000p-18f },
	{ -0x1.36b8000000000p-2f, 0x1.8a9c1e0000000p-18f },
	{ -0x1.2144000000000p-2f, -0x1.5483ae0000000p-20f },
	{ -0x1.0c44000000000p-2f, 0x1.2ae9ea0000000p-18f },
	{ -0x1.ef58000000000p-3f, -0x1.6d96e80000000p-18f },
	{ -0x1.c700000000000p-3f, 0x1.0dc3fc0000000p-21f },
	{ -0x1.9f70000000000p-3f, 0x1.e23fbc0000000p-18f },
	{ -0x1.7898000000000p-3f, -0x1.b4888a0000000p-20f },
	{ -0x1.5280000000000p-3f, 0x1.a1d5e40000000p-19f },
	{ -0x1.2d18000000000p-3f, 0x1.e967980000000p-19f },
	{ -0x1.0858000000000p-3f, -0x1.8e99e40000000p-19f },
	{ -0x1.c880000000000p-4f, -0x1.6116f20000000p-18f },
	{ -0x1.8190000000000p-4f, -0x1.f89d040000000p-18f },
	{ -0x1.3be0000000000p-4f, 0x1.6505c20000000p-21f },
	{ -0x1.eea0000000000p-5f, -0x1.9c20360000000p-20f },
	{ -0x1.67c0000000000p-5f, -0x1.26fdaa0000000p-18f },
	{ -0x1.c640000000000p-6f, 0x1.7c9f5a0000000p-21f },
	{ -0x1.8280000000000p-7f, 0x1.d8fe3c0000000p-18f },
	{ 0x0p+0f, 0x0p+0f },
	{ 0x1.39e0000000000p-5f, 0x1.0dc3fe0000000p-18f },
	{ 0x1.1650000000000p-4f, 0x1.b8851c0000000p-19f },
	{ 0x1.8c30000000000p-4f, 0x1.1978c60000000p-18f },
	{ 0x1.fed0000000000p-4f, -0x1.baf8900000000p-18f },
	{ 0x1.3720000000000p-3f, -0x1.f4f0b80000000p-22f },
	{ 0x1.6d60000000000p-3f, 0x1.f9c33a0000000p-20f },
	{ 0x1.a238000000000p-3f, 0x1.dfff160000000p-18f },
	{ 0x1.d5c0000000000p-3f, 0x1.0a1a7e0000000p-18f },
	{ 0x1.0404000000000p-2f, -0x1.a494b20000000p-18f },
	{ 0x1.1c88000000000p-2f, 0x1.8b369a0000000p-18f },
	{ 0x1.347c000000000p-2f, 0x1.db29880000000p-18f },
	{ 0x1.4be4000000000p-2f, 0x1.f937780000000p-18f },
};
