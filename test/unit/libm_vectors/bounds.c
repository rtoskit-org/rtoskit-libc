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
 * The accuracy contract of the debug, release and speed build types (D47, D48, D57, D98; the fast build type has
 * fast/bounds.c instead, D162, selected by subdir.mk):
 * - to nearest: double and long double below 0.53 ulp of the exact result; float correctly rounded where every
 *   floating type is the FPU's or the double path's (the cores whose <fenv.h> is complete, D98), and below 1 ulp on
 *   the float-float layer of the other cores (D57); a representable exact result exactly;
 * - in the directed rounding modes, where the core has them: float correctly rounded, double within 3 ulp of the
 *   correctly rounded result.
 */

#include <math.h>

#include "libm_vectors.h"

bool libm_exact_result_required([[maybe_unused]] const struct libm_function *f, [[maybe_unused]] int precision_kind, [[maybe_unused]] double want)
{
	return true;
}

bool libm_within_bound(const struct libm_function *f, [[maybe_unused]] int precision_kind, int direction, double got, double want, double error, [[maybe_unused]] int flags, bool float_is_correctly_rounded)
{
	if (direction == 0)
		return f->precision == 24 ? (float_is_correctly_rounded ? got == want : fabs(error) < 1.0) : fabs(error) < 0.53;
	return f->precision == 24 ? got == want : fabs(error) <= 3.0;
}
