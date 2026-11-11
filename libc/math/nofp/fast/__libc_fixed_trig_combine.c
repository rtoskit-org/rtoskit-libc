/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_fixed_trig_combine.c
 *
 * Created on: Nov 11, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdbit.h>
#include <stdint.h>

#include "fixed-fast.h"

/* sin(k pi/32 + s) from s and its terms (fixed-fast.h) */
uint64_t __libc_fixed_trig_combine(int k, int64_t s, int32_t cosine_m1, int32_t g, int *exponent, bool *negative)
{
	static const uint64_t table[9][2] = {
		{ 0x0000000000000000, 0x4000000000000000 },
		{ 0x0645e9af0a6d0af8, 0x3fb11b47a24a4b3c },
		{ 0x0c7c5c1e34d3055b, 0x3ec52f9feeb96056 },
		{ 0x1294062ed59f05a9, 0x3d3e82ad8c5bb4bb },
		{ 0x187de2a6aea962d2, 0x3b20d79e651a8c51 },
		{ 0x1e2b5d3806f63b1e, 0x387165e3017b61a4 },
		{ 0x238e76735cd190d9, 0x3536cc521d434606 },
		{ 0x2899e64a123bac30, 0x317900d62a2e816a },
		{ 0x2d413cccfe779921, 0x2d413cccfe779921 },
	};
	int quadrant = ((k + 8) & 63) / 16;
	int j = ((k + 8) & 15) - 8;
	int a = j < 0 ? -j : j;
	uint64_t magnitude = s < 0 ? -(uint64_t)s : (uint64_t)s;
	int64_t first;
	uint64_t second;
	uint64_t product;
	int64_t value;
	bool second_negative;
	int shift;

	if ((quadrant & 1) == 0) {
		first = (int64_t)table[a][0];
		second = table[a][1];
		if (j < 0)
			first = -first;
		second_negative = s < 0;
		if (a == 0 && magnitude < (UINT64_C(1) << 34))
			return 0;
	} else {
		first = (int64_t)table[a][1];
		second = table[a][0];
		second_negative = (j < 0) == (s < 0);
	}
	{
		uint32_t sh = (uint32_t)(second >> 32);
		uint32_t sl = (uint32_t)second;
		uint32_t th = (uint32_t)(magnitude >> 32);
		uint32_t tl = (uint32_t)magnitude;

		product = (__libc_fixed_wide(sh, th) << 16) + (__libc_fixed_wide(sh, tl) >> 16) + (__libc_fixed_wide(sl, th) >> 16) + (__libc_fixed_wide(sl, tl) >> 48);
	}
	value = (int64_t)product + (__libc_fixed_wide_signed((int32_t)(product >> 30), g) >> 7);
	if (second_negative)
		value = -value;
	value += first + (__libc_fixed_wide_signed((int32_t)(first >> 32), cosine_m1) >> 5);

	*negative = (value < 0) != (quadrant >= 2);
	magnitude = value < 0 ? -(uint64_t)value : (uint64_t)value;
	shift = (int)stdc_leading_zeros(magnitude);
	*exponent = 1 - shift;
	return magnitude << shift;
}
