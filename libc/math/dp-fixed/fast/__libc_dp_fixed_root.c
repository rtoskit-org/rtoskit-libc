/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_dp_fixed_root.c
 *
 * Created on: Nov 14, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>

#include "dp-fixed-fast.h"

/* (a r) >> shift for a below 2^40 and 0 < shift < 96: the 96-bit product from two 32 x 32 products */
static inline uint64_t product_shift(uint64_t a, uint32_t r, int shift)
{
	uint64_t low = __libc_trig_wide((uint32_t)a, r);
	uint64_t high = __libc_trig_wide((uint32_t)(a >> 32), r) + (low >> 32);

	if (shift >= 32)
		return high >> (shift - 32);

	return (high << (32 - shift)) | ((uint32_t)low >> shift);
}

/* s + d r/2^shift with the sign of d, for the remainder corrections */
static inline uint64_t corrected(uint64_t s, int64_t d, uint32_t r, int shift)
{
	uint64_t c = product_shift(d < 0 ? -(uint64_t)d : (uint64_t)d, r, shift);

	return d < 0 ? s - c : s + c;
}

/*
 * sqrt(t 2^-64) 2^64 for t in [2^62, 2^64): r = 1/sqrt(tau) in Q30 from the seed and three Newton steps r (3 - tau r^2)/2
 * (2^-29.5), s = tau r in Q32, then twice s += (t - s^2) r/2 with the remainder exact, in units of 2^-32 (s within a
 * unit) and then of 2^-64 (about 6 units, r's error times the correction)
 */
uint64_t __libc_dp_fixed_root(uint64_t t)
{
	uint32_t w = (uint32_t)(t >> 32);
	uint32_t r = (uint32_t)__libc_dp_fixed_rsqrt_seed[(w >> 27) - 8] << 16;
	uint64_t s;

	for (int i = 0; i < 3; ++i) {
		uint32_t tr = (uint32_t)(__libc_trig_wide(w, r) >> 32);
		uint32_t trr = (uint32_t)(__libc_trig_wide(tr, r) >> 30);

		r = (uint32_t)(__libc_trig_wide(r, 0xc0000000u - trr) >> 31);
	}
	s = __libc_trig_wide(w, r) >> 30;
	if ((s >> 32) != 0)
		s = 0xffffffffu;
	s = corrected(s, (int64_t)(t - s * s), r, 63);
	if ((s >> 32) != 0)
		s = 0xffffffffu;
	s = corrected(s << 32, (int64_t)(t - s * s), r, 31);

	return s;
}
