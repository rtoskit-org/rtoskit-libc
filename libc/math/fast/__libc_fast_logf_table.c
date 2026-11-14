/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fast_logf_table.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math-fast.h"

/*
 * Step i of 2^19 ulps of z from 0x3f330000 (0.6992): 1/c with c the step's midpoint, rounded to double, and
 * -log(1/c) correctly rounded (computed with 200-bit arithmetic; docs/research/libm-fast-float-exp-log.md section 18).
 * Step 9 holds 1, where 1/c = 1 and the logarithm 0, so that r = z - 1 is exact next to 1.
 */
const double __libc_fast_logf_table[16][2] = {
	{ 0x1.661ec79f8f3bep+0, -0x1.57bf7808caadep-2 },
	{ 0x1.571ed4aaf883dp+0, -0x1.2bef0a7c06ddbp-2 },
	{ 0x1.49539f0f010b0p+0, -0x1.01eae7f513a67p-2 },
	{ 0x1.3c995b0b80385p+0, -0x1.b31d8a68224e9p-3 },
	{ 0x1.30d190c8864a5p+0, -0x1.6574f0ac07758p-3 },
	{ 0x1.25e227b0b8ea0p+0, -0x1.1aa2bc79c8100p-3 },
	{ 0x1.1bb4a4a1a343fp+0, -0x1.a4e76ce8c0e5ep-4 },
	{ 0x1.12358f08ae5bap+0, -0x1.1973c5a611cccp-4 },
	{ 0x1.0953f419900a7p+0, -0x1.252f438e10c1ep-5 },
	{ 0x1p+0, 0.0 },
	{ 0x1.e573ae5c66190p-1, 0x1.b42db8ba5c447p-5 },
	{ 0x1.ca4b31f026aa0p-1, 0x1.c5e53aa362eb4p-4 },
	{ 0x1.b2036576afce6p-1, 0x1.526e57720db08p-3 },
	{ 0x1.9c2d163a1aa2dp-1, 0x1.bc2860d224770p-3 },
	{ 0x1.886e6037841edp-1, 0x1.1058bc8a07ee1p-2 },
	{ 0x1.767dcf5534862p-1, 0x1.4043057b6ee09p-2 },
};
