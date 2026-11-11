/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_exp2f_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* 2^(j/32) for j = 0 .. 31 as float pairs (hi, lo), computed here at 40 digits (the trigonometric note, section 15) */
const float __libc_fast_exp2f_table[32][2] = {
	{ 0x1.0p+0f, 0.0f },
	{ 0x1.059b0ep+0f, -0x1.9d4f52p-25f },
	{ 0x1.0b5586p+0f, 0x1.9f3122p-25f },
	{ 0x1.11301ep+0f, -0x1.fdb496p-25f },
	{ 0x1.172b84p+0f, -0x1.c15742p-27f },
	{ 0x1.1d4874p+0f, -0x1.d2e8cap-25f },
	{ 0x1.2387a6p+0f, 0x1.ceac48p-25f },
	{ 0x1.29e9ep+0f, -0x1.5c0424p-25f },
	{ 0x1.306fep+0f, 0x1.4636e2p-25f },
	{ 0x1.371a74p+0f, -0x1.18aac6p-25f },
	{ 0x1.3dea64p+0f, 0x1.824684p-25f },
	{ 0x1.44e086p+0f, 0x1.8624b4p-30f },
	{ 0x1.4bfdaep+0f, -0x1.593abcp-25f },
	{ 0x1.5342b6p+0f, -0x1.2c561p-25f },
	{ 0x1.5ab07ep+0f, -0x1.5bd5ecp-27f },
	{ 0x1.6247ecp+0f, -0x1.f8b55p-25f },
	{ 0x1.6a09e6p+0f, 0x1.9fcef4p-26f },
	{ 0x1.71f75ep+0f, 0x1.1d8beep-25f },
	{ 0x1.7a1148p+0f, -0x1.829fdp-25f },
	{ 0x1.82589ap+0f, -0x1.accc7cp-26f },
	{ 0x1.8ace54p+0f, 0x1.15506ep-27f },
	{ 0x1.93737cp+0f, -0x1.e64744p-25f },
	{ 0x1.9c4918p+0f, 0x1.51f848p-27f },
	{ 0x1.a5503cp+0f, -0x1.b83b54p-25f },
	{ 0x1.ae89fap+0f, -0x1.a94b14p-26f },
	{ 0x1.b7f77p+0f, -0x1.a09438p-25f },
	{ 0x1.c199bep+0f, -0x1.3d56b2p-27f },
	{ 0x1.cb720ep+0f, -0x1.8837ccp-27f },
	{ 0x1.d5818ep+0f, -0x1.822dbcp-27f },
	{ 0x1.dfc974p+0f, -0x1.908c94p-25f },
	{ 0x1.ea4afap+0f, 0x1.52486cp-27f },
	{ 0x1.f50766p+0f, -0x1.246ebp-26f },
};
