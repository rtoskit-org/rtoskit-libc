/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_logf_log2.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* -log(1/c_i) in base 2, as a multiple of 2^-16 and the rest, both computed with 200-bit arithmetic (the note, section 8) */
const float __libc_fast_logf_log2[32][2] = {
	{ -0x1.0014000000000p-1f, -0x1.a856cc0000000p-20f },
	{ -0x1.dfdc000000000p-2f, -0x1.8c2db00000000p-18f },
	{ -0x1.c044000000000p-2f, 0x1.f5400e0000000p-20f },
	{ -0x1.a154000000000p-2f, 0x1.112ca60000000p-18f },
	{ -0x1.8304000000000p-2f, -0x1.ae207c0000000p-19f },
	{ -0x1.6554000000000p-2f, 0x1.4da78e0000000p-18f },
	{ -0x1.4838000000000p-2f, 0x1.a17f8e0000000p-18f },
	{ -0x1.2bac000000000p-2f, 0x1.f75b0a0000000p-18f },
	{ -0x1.0fa8000000000p-2f, -0x1.25a8540000000p-20f },
	{ -0x1.e858000000000p-3f, 0x1.62d64a0000000p-22f },
	{ -0x1.b260000000000p-3f, -0x1.67d4820000000p-22f },
	{ -0x1.7d60000000000p-3f, -0x1.3875380000000p-21f },
	{ -0x1.4950000000000p-3f, 0x1.dace7a0000000p-21f },
	{ -0x1.1628000000000p-3f, 0x1.36a1f40000000p-18f },
	{ -0x1.c7b0000000000p-4f, -0x1.457d720000000p-18f },
	{ -0x1.64d0000000000p-4f, 0x1.c4ded00000000p-20f },
	{ -0x1.0380000000000p-4f, -0x1.f7bdcc0000000p-18f },
	{ -0x1.47a0000000000p-5f, -0x1.3d4b5c0000000p-18f },
	{ -0x1.16c0000000000p-6f, 0x1.da3d620000000p-18f },
	{ 0x0p+0f, 0x0p+0f },
	{ 0x1.c4e0000000000p-5f, -0x1.9fa5260000000p-23f },
	{ 0x1.9190000000000p-4f, -0x1.7982b20000000p-18f },
	{ 0x1.1dd0000000000p-3f, -0x1.7034840000000p-18f },
	{ 0x1.7078000000000p-3f, -0x1.e8fc320000000p-18f },
	{ 0x1.c0d8000000000p-3f, 0x1.b5fb600000000p-18f },
	{ 0x1.0790000000000p-2f, 0x1.5934f20000000p-19f },
	{ 0x1.2db0000000000p-2f, 0x1.0e53860000000p-18f },
	{ 0x1.52dc000000000p-2f, -0x1.1047020000000p-21f },
	{ 0x1.771c000000000p-2f, 0x1.2eb8c40000000p-18f },
	{ 0x1.9a80000000000p-2f, 0x1.1275c20000000p-21f },
	{ 0x1.bd10000000000p-2f, -0x1.9e6f100000000p-19f },
	{ 0x1.ded4000000000p-2f, -0x1.7503960000000p-25f },
};
