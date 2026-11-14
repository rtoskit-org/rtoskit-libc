/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * bounds.c
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The accuracy contract of the fast build type (D162, D163, docs/research/fast-math.md section 4), in place of
 * ../bounds.c (subdir.mk). For the relaxed families, in float, double and long double (which is double, D7):
 * - to nearest: below 1 ulp of the exact result for exp, log, pow, trig and inverse trig, below 1.5 ulp for tan,
 *   the hyperbolic functions and erf; a representable exact result within the same bound, but a zero exactly, sign
 *   included (log(1) = +0, sin(+-0) = +-0);
 * - in the directed rounding modes (x86_64, armv7e-m-dp): no error bound, since the functions compute as if to
 *   nearest; a result that overflows is the one the mode gives (+-inf or the largest finite value), and every
 *   result has the sign of the correctly rounded one.
 * Every other function keeps the speed flavour's contract (../bounds.c), as do, in libm_vectors.c and in every build
 * type, the special values, the exact functions and the exceptions.
 */

#include <math.h>
#include <string.h>

#include "../../include/fast_contract.h"
#include "../libm_vectors.h"

/* The bound of a relaxed function, or 0 for one that keeps the speed flavour's contract */
static double relaxed_bound(const struct libm_function *f, int precision_kind)
{
	/* precision_kind: 0 double, 1 long double (the l suffix), 2 float (the f suffix) */
	return fast_relaxed_bound(f->name, strlen(f->name) - (precision_kind != 0 ? 1 : 0));
}

bool libm_exact_result_required(const struct libm_function *f, int precision_kind, double want)
{
	return relaxed_bound(f, precision_kind) == 0.0 || want == 0.0;
}

bool libm_within_bound(const struct libm_function *f, int precision_kind, int direction, double got, double want, double error, int flags, bool float_is_correctly_rounded)
{
	double bound = relaxed_bound(f, precision_kind);

	if (bound == 0.0) {
		/* The speed flavour's contract, as ../bounds.c */
		if (direction == 0)
			return f->precision == 24 ? (float_is_correctly_rounded ? got == want : fabs(error) < 1.0) : fabs(error) < 0.53;
		return f->precision == 24 ? got == want : fabs(error) <= 3.0;
	}
	if (direction == 0)
		return fabs(error) < bound;
	if ((flags & LIBM_OVERFLOW) != 0)
		return got == want;
	return (flags & LIBM_ZERO_SIGN_ANY) != 0 || !signbit(got) == !signbit(want);
}
