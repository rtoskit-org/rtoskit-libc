/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * atan.c
 *
 * Created on: Nov 13, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <math.h>
#include <stdint.h>

#include "internal/math-fast.h"

/*
 * The fast atan of the double-FPU cores (PLAN D162, D198; docs/research/libm-fast-double.md, section 16): the table
 * kernel for |x| <= 1, and pi/2 - atan(1/|x|) with the reciprocal's remainder above; one rounding. C23 7.12.4.3,
 * F.10.1.3: atan(+-inf) is +-pi/2, as is every |x| from 2^60 once rounded; +-0 returns itself. Where the FPU has a
 * fused multiply-add, atan_fused below.
 */

/* The tiny arguments out of line, so that the fused path needs no stack frame: their helpers make calls */
[[gnu::noinline]] static double atan_tiny(double x)
{
	return __libc_math_tiny(x, -x);
}

/*
 * The fused path (P11.08c, D244, D245; the note, section 39), at the double-FPU cores' 0.9-ulp target: the integer tests
 * on the high word, the j/16 table, and from 1 the table entry nearest 1/a with t = (a c - 1)/(a + c), so that the
 * reciprocal needs no remainder and t rounds once; below 1 t's remainders as in the kernel, which near 1/32 needs them
 * (2.2 ulp without)
 */
static double atan_fused(double x, double a)
{
	uint32_t high = (uint32_t)(__libc_double_bits(a) >> 32);
	bool invert = high >= 0x3ff00000;
	struct __libc_dd sum;
	double base_lo;
	double base;
	double c;
	double d;
	double z;
	double t;
	double v;
	int j;
	const struct __libc_dd *e;

	if (high < 0x3e400000)
		return atan_tiny(x);
	if (high >= 0x43b00000) {
		if (isnan(x))
			return x + x;
		return __libc_fast_signed_sum(__LIBC_PI_2.hi, __LIBC_PI_2.lo, signbit(x) != 0);
	}
	/* c = j/16 nearest a, or from 1 nearest 1/a (rounded: it only picks c) */
	v = invert ? 1.0 / a : a;
	j = (int)(v * 16.0 + 0.5);
	c = (double)j * 0.0625;
	e = &__libc_fast_atan_pairs[j];
	if (invert) {
		/*
		 * atan a = pi/2 - atan c + atan t, t = (c - 1/a)/(1 + c/a) = (a c - 1)/(a + c), a c - 1 rounded once: the base
		 * is at least pi/4 and |t| <= 1/32, so t's own roundings cost below 0.05 ulp
		 */
		t = __builtin_fma(a, c, -1.0) / (a + c);
		sum = __libc_dd_fast_sum(__LIBC_PI_2.hi, -e->hi);
		base = sum.hi;
		base_lo = sum.lo + (__LIBC_PI_2.lo - e->lo);
	} else {
		/*
		 * atan a = atan c + atan t, t = (a - c)/(1 + a c), a - c exact: near 1/32 t is half the result, so 1 + a c's
		 * rounding error and the quotient's remainder go in base_lo, each by a fused multiply-add
		 */
		d = __builtin_fma(a, c, 1.0);
		t = (a - c) / d;
		base = e->hi;
		base_lo = e->lo + __builtin_fma(-t, __builtin_fma(a, c, 1.0 - d), __builtin_fma(-t, d, a - c)) / d;
	}
	/* |t| <= 1/32: atan t = t + t z P(z), the kernel's five terms (2^-65.6 relative to t); base + t by a fast two-sum */
	z = t * t;
	sum = __libc_dd_fast_sum(base, t);

	return __libc_fast_signed_sum(sum.hi, sum.lo + (base_lo + t * (z * (-0x1.5555555555555p-2 + z * (0x1.99999999998a4p-3 + z * (-0x1.24924922a7020p-3 + z * (0x1.c71c46c7ecbacp-4 + z * -0x1.739871480f801p-4)))))), signbit(x) != 0);
}

double atan(double x)
{
	double a = __builtin_fabs(x);
	double hi;
	double lo;

	if (__libc_fpu_fma)
		return atan_fused(x, a);

	/* A NaN compares quietly (isless): the relational operators would raise invalid */
	if (isless(a, 0x1p-27))
		return __libc_math_tiny(x, -x);
	if (!isless(a, 0x1p60)) {
		if (isnan(x))
			return x + x;
		return __libc_fast_signed_sum(__LIBC_PI_2.hi, __LIBC_PI_2.lo, signbit(x) != 0);
	}

	if (a <= 1.0) {
		hi = __libc_fast_atan_kernel(a, 0.0, &lo);
	} else {
		double u_lo;
		double u = __libc_fast_quotient(1.0, a, &u_lo);
		double k_lo;
		double k = __libc_fast_atan_kernel(u, u_lo, &k_lo);
		struct __libc_dd sum = __libc_dd_fast_sum(__LIBC_PI_2.hi, -k);

		hi = sum.hi;
		lo = (sum.lo + __LIBC_PI_2.lo) - k_lo;
	}

	return __libc_fast_signed_sum(hi, lo, signbit(x) != 0);
}
