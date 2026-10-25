/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cfcmp-thumb1.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "softsp.h"

/*
 * On Armv6-M the flag-returning comparisons are libgcc's Thumb-1 ones, which
 * do not give RTABI's flags (they end with cmp/cmn of __lesf2's result: C is
 * set for less and clear for equal). GCC never calls them on Thumb-1, which
 * uses the 0/1 forms, and the library does not define them there (PLAN D109).
 */
void check_flag_compare([[maybe_unused]] uint32_t a, [[maybe_unused]] uint32_t b)
{
}
