/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_atanf_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* atan(j/8) and its remainder, j = 0 .. 8, computed with 200-bit arithmetic (the trigonometric note, section 7) */
const float __libc_fast_atanf_table[9][2] = {
	{ 0.0f, 0.0f },
	{ 0x1.fd5baap-4f, -0x1.54f424p-30f },
	{ 0x1.f5b760p-3f, -0x1.b4dfc8p-29f },
	{ 0x1.6f6194p-2f, 0x1.e4def0p-30f },
	{ 0x1.dac670p-2f, 0x1.586ed4p-28f },
	{ 0x1.1e00bap-1f, 0x1.7bdfd6p-26f },
	{ 0x1.4978fap-1f, 0x1.934f70p-28f },
	{ 0x1.700a7cp-1f, 0x1.5e118cp-27f },
	{ 0x1.921fb6p-1f, -0x1.777a5cp-26f },
};
