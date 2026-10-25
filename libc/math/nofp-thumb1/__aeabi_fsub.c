/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fsub.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/aeabi.h"

/* a - b is a + (-b) (IEEE 754-2019 5.4.1); see __aeabi_dsub (PLAN D55) */
float __aeabi_fsub(float a, float b)
{
	return __aeabi_fadd(a, -b);
}
