/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * powf.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The special cases of C23 F.10.4.5: a zero, infinite or NaN x or y, or a negative x to a non-integer power, all of
 * which __libc_fast_powf_special decides (the NaN is never returned)
 */
[[gnu::noinline]] static float powf_special(float x, float y)
{
	float result = NAN;
	bool negative;

	__libc_fast_powf_special(x, y, &result, &negative);
	return result;
}

/* |t| beyond 1000: the helpers give the mode's result and raise the exception (a constant would fold) */
[[gnu::noinline]] static float powf_far(double t, double sign)
{
	return (float)(t > 0 ? __libc_math_overflow(sign) : __libc_math_underflow(sign));
}

/*
 * A finite nonzero y's bits as an integer kind, without a call (truncf would make the common path keep a frame): -1
 * not an integer, 1 odd, 0 even (every |y| from 2^24 up is even, every one below 1 is not an integer)
 */
static inline int powf_kind(uint32_t iy)
{
	int e = (int)((iy >> 23) & 0xff) - 127;

	if (e < 0)
		return -1;
	if (e > 23)
		return 0;
	if ((iy & (0x007fffffu >> e)) != 0)
		return -1;
	return e == 0 ? 1 : (int)((iy >> (23 - e)) & 1);
}

/*
 * The fast powf of the double-FPU cores (PLAN D162, D170, P11.06b; the note, sections 10 and 18): 2^(y log2 |x|) in
 * double, rounded once to float, below 0.53 ulp to nearest. log2 |x| = k + (log c + log1p r) / ln2 with
 * __libc_fast_logf_step and __libc_fast_powf_log1p (degree 6, 2^-37.92 relative, so t = y log2 |x| is within 2^-29.6
 * for |t| up to 150), then __libc_fast_exp2. One test sends x outside the positive normal floats, and y zero, infinite
 * or a NaN, off the common path; a negative x to an integer power comes back to it with the result's sign, a
 * subnormal |x| scaled by 2^23. x = 1 and y = 1 need no test (log2 1 = 0 and 2^0 = 1 exactly; 2^(log2 x) rounds to x).
 */
float powf(float x, float y)
{
	uint32_t ix = __libc_float_bits(x);
	uint32_t iy = __libc_float_bits(y);
	double sign = 1.0;
	double r;
	double l;
	double t;
	int k;

	if (__builtin_expect(ix - 0x00800000u >= 0x7f800000u - 0x00800000u || (iy << 1) - 1u >= 0xff000000u - 1u, 0)) {
		/* Finite nonzero x and y, x negative only to an integer power: else the special cases */
		if ((ix << 1) - 1u >= 0xff000000u - 1u || (iy << 1) - 1u >= 0xff000000u - 1u || (ix >> 31 && powf_kind(iy) < 0))
			return powf_special(x, y);
		if (ix >> 31 && powf_kind(iy) == 1)
			sign = -1.0;
		ix &= 0x7fffffffu;
		if (ix < 0x00800000u)
			/* Scaled by 2^23, less 23 2^23 so that __libc_fast_logf_step finds k - 23 */
			ix = __libc_float_bits(__libc_float_from(ix) * 0x1p23f) - (23u << 23);
	}
	l = __libc_fast_logf_step(ix, &r, &k);
	l += __libc_fast_powf_log1p(r);
	t = (double)y * ((double)k + l * 0x1.71547652b82fep+0);
	if (__builtin_fabs(t) > 1000.0)
		return powf_far(t, sign);
	return (float)(sign * __libc_fast_exp2(t));
}
