/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_ui2f.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "softfloat.h"

/*
 * (float)a for an unsigned int (C23 6.3.1.4p2, RTABI 4.1.2; PLAN D109):
 * round to nearest, ties to even (D44).
 */
__LIBC_AEABI float __aeabi_ui2f(unsigned int a)
{
	return __libc_softfloat_from(__libc_softfloat_from_u32(0, a));
}
