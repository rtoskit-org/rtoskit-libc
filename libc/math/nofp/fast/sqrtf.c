/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sqrtf.c
 *
 * Created on: Nov 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/*
 * The fast sqrtf of the cores without an FPU (PLAN D162, D242; docs/research/libm-fast-double.md section 38), still
 * correctly rounded (C23 F.3, D222): x = m 2^E with m of 24 bits (subnormals normalised), a = 2m or 4m by E's parity
 * (25 or 26 bits), and the 25-bit root of a 2^24 digit by digit: 13 steps that take two of a's bits each, then 12 that take
 * zeros, the remainder below 2^27. The 25th bit rounds: a root is never half way between two floats. The multiplies of a
 * Newton root cost more than the digits on Armv6-M (417 against 330 instructions, measured), and a quarter of the size.
 */
float sqrtf(float x)
{
	uint32_t bits = __libc_float_bits(x);
	uint32_t m = bits & 0x007fffffu;
	int32_t e = (int32_t)(bits >> 23);
	uint32_t radicand;
	uint32_t remainder = 0;
	uint32_t root = 0;
	uint32_t trial;

	/* +-0, +infinity and NaN: x itself (a NaN quietened); a negative number is invalid */
	if ((bits << 1) == 0 || bits >= 0x7f800000u)
		return bits > 0x80000000u && bits <= 0xff800000u ? __libc_math_invalidf() : x + x;
	if (e == 0) {
		int shift = (int)stdc_leading_zeros(m) - 8;

		m <<= shift;
		e = 1 - shift;
	} else {
		m |= 0x00800000u;
	}
	/* E = e - 127; a = m << 1 (E even: e odd) or m << 2 (E odd), top-aligned for the digits */
	radicand = m << ((e & 1) != 0 ? 7 : 8);
	/*
	 * The radicand's top two bits are not both zero, so the root has k bits after k steps: the loops end on its size, which
	 * costs one comparison a step in place of a counter's two
	 */
	do {
		remainder = (remainder << 2) | (radicand >> 30);
		radicand <<= 2;
		trial = (root << 2) | 1;
		root <<= 1;
		if (remainder >= trial) {
			remainder -= trial;
			root |= 1;
		}
	} while (root < 0x1000u);
	do {
		remainder <<= 2;
		trial = (root << 2) | 1;
		root <<= 1;
		if (remainder >= trial) {
			remainder -= trial;
			root |= 1;
		}
	} while (root < 0x1000000u);

	/* sqrt(x) = root 2^-24 2^((E + 2) >> 1) with root of 25 bits: the exponent field (E >> 1) + 127, the leading bit adding 1 */
	return __libc_float_from((uint32_t)((((e - 127) >> 1) + 126) << 23) + (root >> 1) + (root & 1));
}
