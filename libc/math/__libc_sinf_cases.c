/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_sinf_cases.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/math.h"

/*
 * The arguments whose double result is a float halfway point while the exact
 * value is not (PLAN D98), shared by sinf and sincosf (P8.02)
 */
const struct __libc_float_case __libc_sinf_cases[__LIBC_SINF_CASES] = {
	{ 0xc6199998, 0x3eb1fa5d }, /* -0x1.33333p+13 -> 0x1.63f4bap-2 */
	{ 0x46199998, 0xbeb1fa5d }, /* 0x1.33333p+13 -> -0x1.63f4bap-2 */
};
