/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_l2f.c
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "internal/aeabi.h"
#include "softfloat.h"

/*
 * (float)a for a long long (C23 6.3.1.4p2, RTABI 4.1.2; PLAN D109): the
 * sign and the magnitude, LLONG_MIN's being 2^63 in unsigned arithmetic.
 */
__LIBC_AEABI float __aeabi_l2f(long long a)
{
	uint32_t sign = a < 0 ? __LIBC_SOFTFLOAT_SIGN : 0;
	uint64_t magnitude = a < 0 ? 0u - (uint64_t)a : (uint64_t)a;

	return __libc_softfloat_from(__libc_softfloat_from_u64(sign, magnitude));
}
