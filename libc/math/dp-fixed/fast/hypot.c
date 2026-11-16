/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * hypot.c
 *
 * Created on: Nov 16, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "dp-fixed-fast.h"
#include "internal/math.h"

/* A 128-bit unsigned integer, hi:lo */
struct hypot_wide {
	uint64_t hi;
	uint64_t lo;
};

/* m^2 exactly for m below 2^55: m = h 2^32 + l, h^2 2^64 + 2 h l 2^32 + l^2 (2 h l below 2^56) */
static inline struct hypot_wide hypot_square(uint64_t m)
{
	uint32_t l = (uint32_t)m;
	uint32_t h = (uint32_t)(m >> 32);
	uint64_t ll = __libc_trig_wide(l, l);
	uint64_t hl = __libc_trig_wide(h, l);
	uint64_t lo = ll + (hl << 33);

	return (struct hypot_wide){ .hi = __libc_trig_wide(h, h) + (hl >> 31) + (lo < ll), .lo = lo };
}

/* x >> n for 0 <= n < 128, *sticky set when a one bit is lost */
static inline struct hypot_wide hypot_shift_right(struct hypot_wide x, int n, bool *sticky)
{
	if (n == 0)
		return x;
	if (n < 64) {
		*sticky |= (x.lo << (64 - n)) != 0;
		return (struct hypot_wide){ .hi = x.hi >> n, .lo = (x.lo >> n) | (x.hi << (64 - n)) };
	}
	*sticky |= x.lo != 0 || (n > 64 && (x.hi << (128 - n)) != 0);

	return (struct hypot_wide){ .hi = 0, .lo = n == 64 ? x.hi : x.hi >> (n - 64) };
}

/* x << n for 0 <= n < 128, the result below 2^128 */
static inline struct hypot_wide hypot_shift_left(struct hypot_wide x, int n)
{
	if (n == 0)
		return x;
	if (n >= 64)
		return (struct hypot_wide){ .hi = x.lo << (n - 64), .lo = 0 };

	return (struct hypot_wide){ .hi = (x.hi << n) | (x.lo >> (64 - n)), .lo = x.lo << n };
}

/* The significand of a nonzero finite magnitude's bits as m in [2^52, 2^53) with the value m 2^*e, subnormals normalized */
static inline uint64_t hypot_significand(uint64_t bits, int32_t *e)
{
	int k;

	if (bits >= 0x0010000000000000ull) {
		*e = (int32_t)(bits >> 52) - 1075;
		return (bits & 0x000fffffffffffffull) | 0x0010000000000000ull;
	}
	k = __builtin_clzll(bits) - 11;
	*e = -1074 - k;

	return bits << k;
}

/*
 * The rounding of the root at bit 11 + p of __libc_dp_fixed_root's value (p from 0 for a normal result, more for a
 * subnormal one or for S above 2^106) when it lies within 32 units of a midpoint: the candidate q below it, and the
 * midpoint's square against W = 4 S exactly, (2 q + 1)^2 4^p, W carrying the sticky part of b's square below its last
 * bit. Ties to even.
 */
[[gnu::noinline]] static uint64_t hypot_decide(uint64_t q, int p, struct hypot_wide w, bool sticky)
{
	struct hypot_wide m = hypot_shift_left(hypot_square(2 * q + 1), 2 * p);

	if (m.hi != w.hi || m.lo != w.lo || sticky)
		return (m.hi < w.hi || (m.hi == w.hi && m.lo <= w.lo)) ? q + 1 : q;

	return q + (q & 1);
}

/* The ends: an infinity wins even over a NaN (C23 F.10.4.4), then a NaN; a zero b, or b below 2^-61 a, gives a */
[[gnu::noinline]] static double hypot_ends(uint64_t ia, uint64_t ib)
{
	if (ia == 0x7ff0000000000000ull || ib == 0x7ff0000000000000ull)
		return INFINITY;
	if (ia > 0x7ff0000000000000ull)
		return __libc_double_from(ia) + __libc_double_from(ib);

	return __libc_double_from(ia);
}

/*
 * The fast hypot of the soft-double cores (PLAN D162, D219), correctly rounded in 64-bit integers with no soft-double
 * operation on the common path (docs/research/libm-fast-double.md, section 30). a >= b > 0 as m 2^e with m in [2^52,
 * 2^53) (b below 2^-61 a gives a: b^2/(2 a^2) is below 2^-123). W = 4 S, S = ma^2 + mb^2 4^-(ea - eb), exactly but for
 * a sticky bit (b's square shifted by 2 (ea - eb) - 2, so that W keeps S's two fractional bits), in [2^106, 2^110). The
 * root of W's top 64 bits by __libc_dp_fixed_root is sqrt(S) 2^11 (S below 2^106) or sqrt(S/4) 2^11 (above, the
 * exponent one higher), within about 8 units; rounded at its bit 11 (more for a subnormal result), and decided exactly by
 * hypot_decide within 32 units of a midpoint. These cores round to nearest and raise no flags (D44). C23 7.12.7.4.
 */
double hypot(double x, double y)
{
	uint64_t ia = __libc_double_bits(x) & 0x7fffffffffffffffull;
	uint64_t ib = __libc_double_bits(y) & 0x7fffffffffffffffull;
	struct hypot_wide w;
	struct hypot_wide b2;
	bool sticky = false;
	uint64_t ma;
	uint64_t mb;
	uint64_t root;
	uint64_t q;
	uint64_t r;
	uint64_t half;
	int32_t ea;
	int32_t eb;
	int32_t biased;
	int shift = 11;
	int p;

	if (ia < ib) {
		uint64_t swap = ia;

		ia = ib;
		ib = swap;
	}
	if (ia >= 0x7ff0000000000000ull || ib == 0)
		return hypot_ends(ia, ib);
	ma = hypot_significand(ia, &ea);
	mb = hypot_significand(ib, &eb);
	if (ea - eb > 61)
		return hypot_ends(ia, ib);

	w = hypot_shift_left(hypot_square(ma), 2);
	b2 = hypot_square(mb);
	if (ea == eb)
		b2 = hypot_shift_left(b2, 2);
	else
		b2 = hypot_shift_right(b2, 2 * (ea - eb) - 2, &sticky);
	w.lo += b2.lo;
	w.hi += b2.hi + (w.lo < b2.lo);

	/* W's top 64 bits, in [2^62, 2^64); above 2^108 the root is that of S/4 and the result's exponent one higher */
	p = w.hi >= (1ull << 44);
	root = __libc_dp_fixed_root(p != 0 ? (w.hi << 18) | (w.lo >> 46) : (w.hi << 20) | (w.lo >> 44));
	biased = ea + p + 1075;
	/* A subnormal result: rounded that many bits higher, the exponent field 0 */
	if (biased < 1) {
		shift += 1 - biased;
		p += 1 - biased;
		biased = 1;
	}
	q = root >> shift;
	r = root & ((1ull << shift) - 1);
	half = 1ull << (shift - 1);
	if (r + 32 - half <= 64)
		q = hypot_decide(q, p, w, sticky);
	else
		q += r > half;

	/* q in [2^52, 2^53] for a normal result (2^53 carries into the exponent), q itself for a subnormal one */
	q += (uint64_t)(biased - 1) << 52;
	if (q >= 0x7ff0000000000000ull)
		return __libc_math_overflow(1.0);

	return __libc_double_from(q);
}
