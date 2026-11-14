/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_atan_table.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* atan(j/8), j = 0 .. 8, computed with 200-bit arithmetic (the trigonometric note, section 6) */
const double __libc_fast_atan_table[9] = {
	0.0,
	0x1.fd5ba9aac2f6ep-4,
	0x1.f5b75f92c80ddp-3,
	0x1.6f61941e4def1p-2,
	0x1.dac670561bb4fp-2,
	0x1.1e00babdefeb4p-1,
	0x1.4978fa3269ee1p-1,
	0x1.700a7c5784634p-1,
	0x1.921fb54442d18p-1,
};
