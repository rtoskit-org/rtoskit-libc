/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_cosf_cases.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math.h"

/*
 * The arguments whose double result is a float halfway point while the exact
 * value is not (PLAN D98), shared by cosf and sincosf (P8.02)
 */
const struct __libc_float_case __libc_cosf_cases[__LIBC_COSF_CASES] = {
	{ 0x5f18b878, 0x3f7f14bb }, /* 0x1.3170fp+63 -> 0x1.fe2976p-1 */
	{ 0xdf18b878, 0x3f7f14bb }, /* -0x1.3170fp+63 -> 0x1.fe2976p-1 */
	{ 0xe115cb11, 0x3f78142f }, /* -0x1.2b9622p+67 -> 0x1.f0285ep-1 */
	{ 0x6115cb11, 0x3f78142f }, /* 0x1.2b9622p+67 -> 0x1.f0285ep-1 */
};
