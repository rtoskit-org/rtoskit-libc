/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_sinf_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* sin(j pi/32), its remainder, cos(j pi/32), its remainder, j = 0 .. 8, computed with 300-bit arithmetic (the note, section 4) */
const float __libc_fast_sinf_table[9][4] = {
	{ 0.0f, 0.0f, 0x1p0f, 0.0f },
	{ 0x1.917a6cp-4f, -0x1.eb25eap-31f, 0x1.fd88dap-1f, 0x1.e89292p-28f },
	{ 0x1.8f8b84p-3f, -0x1.cb2cfap-30f, 0x1.f6297cp-1f, 0x1.feeb96p-26f },
	{ 0x1.294062p-2f, 0x1.dab3e0p-27f, 0x1.e9f416p-1f, -0x1.273a44p-26f },
	{ 0x1.87de2ap-2f, 0x1.abaa58p-28f, 0x1.d906bcp-1f, 0x1.e651a8p-26f },
	{ 0x1.e2b5d4p-2f, -0x1.fe4272p-28f, 0x1.c38b30p-1f, -0x1.cfe84ap-26f },
	{ 0x1.1c73b4p-1f, -0x1.9465cep-27f, 0x1.a9b662p-1f, 0x1.21d434p-26f },
	{ 0x1.44cf32p-1f, 0x1.424776p-27f, 0x1.8bc806p-1f, 0x1.62a2e8p-26f },
	{ 0x1.6a09e6p-1f, 0x1.9fcef4p-27f, 0x1.6a09e6p-1f, 0x1.9fcef4p-27f },
};
