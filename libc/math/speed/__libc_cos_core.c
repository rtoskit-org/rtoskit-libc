/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_cos_core.c
 *
 * Created on: Nov 17, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math.h"

/*
 * The speed flavour's cos core (PLAN D228): the series with both squares exact. Two exact products in place of
 * the size flavour's four (1 - 2 sin^2(r/2) through the sin core) make sin and cos about 12 % faster on Arm, for
 * about 750 more bytes on the cores without a double FPU.
 *
 * cos(r) for |r| <= 0.8: 1 - r^2/2 + r^4/24 with both squares exact, so the
 * part left to plain double arithmetic, r^6 p(r^2), stays below 2^-11.5. The
 * r^4/24 term, up to 2^-6, cannot be rounded as a double (2^-60, 0.008 ulp):
 * 1/24 is 1/32 + 1/128 + 1/512 + 1/1536, the first three exact scalings
 * summed exactly, and only r^4/1536 (below 2^-12) is rounded, with the tail.
 * p is a near-minimax fit of six terms (2^-74 on |r| <= 0.8). The low part y
 * of r enters through y sin(r) ~ y r (1 - r^2/6 + r^4/120).
 */
struct __libc_dd __libc_cos_core(struct __libc_dd r)
{
	double x = r.hi;
	double z = x * x;
	struct __libc_dd square = __libc_dd_product(x, x);
	struct __libc_dd fourth = __libc_dd_product(square.hi, square.hi);
	double tail = z * z * z * (-0x1.6c16c16c16c14p-10 + z * (0x1.a01a01a019530p-16 + z * (-0x1.27e4fb765ed98p-22 + z * (0x1.1eed8cf977dc8p-29 + z * (-0x1.9393843b348bbp-37 + z * 0x1.aae026013157cp-45)))));
	struct __libc_dd half = __libc_dd_fast_sum(1.0, -0.5 * square.hi);
	struct __libc_dd first = __libc_dd_fast_sum(half.hi, 0x1p-5 * fourth.hi);
	struct __libc_dd second = __libc_dd_fast_sum(first.hi, 0x1p-7 * fourth.hi);
	struct __libc_dd third = __libc_dd_fast_sum(second.hi, 0x1p-9 * fourth.hi);
	struct __libc_dd rest = __libc_dd_fast_sum(third.hi, fourth.hi * 0x1.5555555555555p-11 + tail);
	double low = half.lo + first.lo + second.lo + third.lo - 0.5 * square.lo + 0x1.5555555555555p-5 * (fourth.lo + 2.0 * square.hi * square.lo);

	return __libc_dd_fast_sum(rest.hi, rest.lo + low - x * r.lo * (1.0 - z * (0x1.5555555555555p-3 - z * 0x1.1111111111111p-7)));
}
