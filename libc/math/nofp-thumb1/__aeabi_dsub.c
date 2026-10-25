/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_dsub.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/aeabi.h"

/*
 * a - b is a + (-b) (IEEE 754-2019 5.4.1), with the same result, sign of
 * zero and exceptions, so the addition helper serves both. libgcc's soft-fp
 * build for Armv6-M keeps a separate 1.4 KB subtraction instead (PLAN D55).
 * The negation flips the sign bit only; it is not a floating-point operation.
 */
double __aeabi_dsub(double a, double b)
{
	return __aeabi_dadd(a, -b);
}
