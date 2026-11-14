/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pow.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * (log1p(r) - r + r^2/2)/r^3 for |r| <= 2^-7.47 by its Chebyshev fit of degree 6 in double (2^-54 relatively, the
 * rounding of 1/3 itself: times |y log|x|| at most 745 and r^2/3, below 2^-61 of the result; degree 5 measured 0.559 ulp)
 */
static inline double pow_log_q(double r, double r2)
{
	return 0x1.5555555555555p-2 + r * -0x1.0000000000006p-2 + r2 * (0x1.99999999999a5p-3 + r * -0x1.555555524a540p-3 + r2 * (0x1.24924921ce195p-3 + r * -0x1.0002eb43a3fc0p-3 + r2 * 0x1.c721c070d115fp-4));
}

/*
 * e^(zh + zl) with z = zh + zl beyond 700 in magnitude: overflow above ln(DBL_MAX), underflow below exp's end (D187),
 * between them the scaled path (__libc_fast_exp_scale), the sign after its rounding
 */
[[gnu::noinline]] static double pow_exp_ends(double z, double zh, double zl, uint64_t sign)
{
	double s = sign != 0 ? -1.0 : 1.0;
	double result;
	double kd;
	int32_t k;
	double u;

	if (z > 0x1.62e42fefa39efp+9)
		return __libc_math_overflow(s);
	if (z < -0x1.74910d52d3051p+9)
		return __libc_math_underflow(s);
	k = __libc_fast_exp_index(z * 0x1.71547652b82fep+6, &kd);
	u = ((zh - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39) + zl;
	result = __libc_fast_exp_scale(k, __libc_fast_expm1_poly(u));

	return s * result;
}

/*
 * The fast pow of the double-FPU cores (PLAN D162, D193): e^(y log|x|) with one rounding. log|x| as hi + lo from the
 * logarithms' table (D190: r = z invc - 1 exact, k ln 2_hi + logc exact), r^2/2 exact by a fused multiply-add (or
 * Dekker's product), r^3 Q(r) and the tails in lo (unnormalized: up to 2^-16 of hi): its error times |y| is at most
 * 2^-58 of the result. y (hi + lo) as a
 * pair (the product's error exact), then exp's reduction of the pair (the low part added to the reduced argument) and
 * its table and polynomial (D187), the sign in the scale, so that only the last addition rounds. Out of line: the special
 * cases that need no logarithm, and |y log|x|| beyond 700 (the ends and the scaled path). A negative x with an integer
 * y and a subnormal x (scaled by 2^52, the exponent taken back) are fixed in line.
 */
double pow(double x, double y)
{
	const struct __libc_fast_log_entry *entry;
	uint64_t ix = __libc_double_bits(x);
	uint64_t iy = __libc_double_bits(y);
	const uint64_t *pair;
	struct __libc_dd product;
	uint64_t sign = 0;
	double scale;
	double kd;
	double r2;
	double t1;
	double t2;
	double hi;
	double lo;
	double zh;
	double zl;
	double w;
	double z;
	double r;
	double u;
	double p;
	int32_t k;

	if (((ix >> 52) - 0x001) >= 0x7fe || ((iy >> 52) & 0x7ff) - 0x3be >= 0x43e - 0x3be) {
		/* Here only a negative finite x or a subnormal one with y of 2^-65 to 2^63: the rest needs no logarithm */
		if (((iy >> 52) & 0x7ff) - 0x3be >= 0x43e - 0x3be || (ix << 1) == 0 || ((ix >> 52) & 0x7ff) == 0x7ff || x == 1.0)
			return __libc_fast_pow_special(x, y);
		if ((ix >> 63) != 0) {
			int kind = __libc_fast_pow_integer_kind(iy);

			if (kind == 0)
				return __libc_fast_pow_special(x, y);
			sign = kind == 1 ? 0x8000000000000000ull : 0;
			ix &= 0x7fffffffffffffffull;
		}
		if (ix < 0x0010000000000000ull)
			ix = __libc_double_bits(__libc_double_from(ix) * 0x1p52) - (52ull << 52);
	}

	entry = __libc_fast_log_reduce(__libc_fast_log_table, ix, &z, &k);
	r = __libc_fast_log_r(z, entry->invc);
	kd = (double)k;
	w = kd * 0x1.62e42fefa3800p-1 + entry->logc;
	t1 = w + r;
	product = __libc_dd_product(r, r);
	p = -0.5 * product.hi;
	t2 = t1 + p;
	r2 = product.hi;
	lo = ((w - t1) + r) + ((t1 - t2) + p) + (-0.5 * product.lo + r * r2 * pow_log_q(r, r2) + (kd * 0x1.ef35793c76730p-45 + entry->tail));
	hi = t2;

	product = __libc_dd_product(y, hi);
	zh = product.hi;
	zl = product.lo + y * lo;
	/* lo carries r^3 Q, up to 2^-16 of hi: the index from zh + zl keeps the reduced argument within exp's range */
	z = zh + zl;
	/* Below 700 in magnitude the result is normal and the scale's exponent within its field */
	if (!(__builtin_fabs(z) <= 700.0))
		return pow_exp_ends(z, zh, zl, sign);

	k = __libc_fast_exp_index(z * 0x1.71547652b82fep+6, &kd);
	u = ((zh - kd * 0x1.62e42fee00000p-7) - kd * 0x1.a39ef35793c76p-39) + zl;
	p = __libc_fast_expm1_poly(u);
	pair = __libc_fast_exp_entry(k);
	scale = __libc_double_from(__libc_fast_exp_scale_bits(pair, k >> 6) | sign);

	return scale + scale * (__libc_double_from(pair[1]) + p);
}
