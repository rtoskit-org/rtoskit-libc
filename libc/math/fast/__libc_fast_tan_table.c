/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_tan_table.c
 *
 * Created on: Nov 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/* tan(j pi/32) for j = 0..8, each as the double nearest it and the double nearest the rest (P11.08d3, D248) */
const struct __libc_dd __libc_fast_tan_table[9] = {
	{ 0x0.0p+0, 0x0.0p+0 },
	{ 0x1.936bb8c5b2da2p-4, 0x1.87d054f689d7ap-58 },
	{ 0x1.975f5e0553158p-3, 0x1.ef5d367441946p-61 },
	{ 0x1.36a08355c63dcp-2, -0x1.d704d1bfdb6e8p-57 },
	{ 0x1.a827999fcef32p-2, 0x1.08b2fb1366ea9p-56 },
	{ 0x1.11ab7190834ecp-1, -0x1.e564bcd1e635dp-55 },
	{ 0x1.561b82ab7f990p-1, 0x1.7a8c52172b675p-55 },
	{ 0x1.a43002ae42850p-1, -0x1.a6db80fe796fep-56 },
	{ 0x1.0000000000000p+0, -0x1.0000000000000p-300 },
};
