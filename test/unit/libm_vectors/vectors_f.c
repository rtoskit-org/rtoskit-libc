/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * vectors_f.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* The float vectors bound to the f functions (long double is binary64, PLAN D7, so it shares the double vectors) */
#include <math.h>

#include "libm_vectors.h"

#include "vectors-float.inc"

const struct libm_function libm_float_functions[] = {
	{ "acosf", LIBM_UNARY, acos_vectors, sizeof(acos_vectors) / sizeof(acos_vectors[0]), 24, (const void *)&acosf },
	{ "asinf", LIBM_UNARY, asin_vectors, sizeof(asin_vectors) / sizeof(asin_vectors[0]), 24, (const void *)&asinf },
	{ "atanf", LIBM_UNARY, atan_vectors, sizeof(atan_vectors) / sizeof(atan_vectors[0]), 24, (const void *)&atanf },
	{ "atan2f", LIBM_BINARY, atan2_vectors, sizeof(atan2_vectors) / sizeof(atan2_vectors[0]), 24, (const void *)&atan2f },
	{ "cosf", LIBM_UNARY, cos_vectors, sizeof(cos_vectors) / sizeof(cos_vectors[0]), 24, (const void *)&cosf },
	{ "sinf", LIBM_UNARY, sin_vectors, sizeof(sin_vectors) / sizeof(sin_vectors[0]), 24, (const void *)&sinf },
	{ "tanf", LIBM_UNARY, tan_vectors, sizeof(tan_vectors) / sizeof(tan_vectors[0]), 24, (const void *)&tanf },
	{ "acoshf", LIBM_UNARY, acosh_vectors, sizeof(acosh_vectors) / sizeof(acosh_vectors[0]), 24, (const void *)&acoshf },
	{ "asinhf", LIBM_UNARY, asinh_vectors, sizeof(asinh_vectors) / sizeof(asinh_vectors[0]), 24, (const void *)&asinhf },
	{ "atanhf", LIBM_UNARY, atanh_vectors, sizeof(atanh_vectors) / sizeof(atanh_vectors[0]), 24, (const void *)&atanhf },
	{ "coshf", LIBM_UNARY, cosh_vectors, sizeof(cosh_vectors) / sizeof(cosh_vectors[0]), 24, (const void *)&coshf },
	{ "sinhf", LIBM_UNARY, sinh_vectors, sizeof(sinh_vectors) / sizeof(sinh_vectors[0]), 24, (const void *)&sinhf },
	{ "tanhf", LIBM_UNARY, tanh_vectors, sizeof(tanh_vectors) / sizeof(tanh_vectors[0]), 24, (const void *)&tanhf },
	{ "expf", LIBM_UNARY, exp_vectors, sizeof(exp_vectors) / sizeof(exp_vectors[0]), 24, (const void *)&expf },
	{ "exp2f", LIBM_UNARY, exp2_vectors, sizeof(exp2_vectors) / sizeof(exp2_vectors[0]), 24, (const void *)&exp2f },
	{ "exp10f", LIBM_UNARY, exp10_vectors, sizeof(exp10_vectors) / sizeof(exp10_vectors[0]), 24, (const void *)&exp10f },
	{ "expm1f", LIBM_UNARY, expm1_vectors, sizeof(expm1_vectors) / sizeof(expm1_vectors[0]), 24, (const void *)&expm1f },
	{ "logf", LIBM_UNARY, log_vectors, sizeof(log_vectors) / sizeof(log_vectors[0]), 24, (const void *)&logf },
	{ "log2f", LIBM_UNARY, log2_vectors, sizeof(log2_vectors) / sizeof(log2_vectors[0]), 24, (const void *)&log2f },
	{ "log10f", LIBM_UNARY, log10_vectors, sizeof(log10_vectors) / sizeof(log10_vectors[0]), 24, (const void *)&log10f },
	{ "log1pf", LIBM_UNARY, log1p_vectors, sizeof(log1p_vectors) / sizeof(log1p_vectors[0]), 24, (const void *)&log1pf },
	{ "cbrtf", LIBM_UNARY, cbrt_vectors, sizeof(cbrt_vectors) / sizeof(cbrt_vectors[0]), 24, (const void *)&cbrtf },
	{ "sqrtf", LIBM_UNARY, sqrt_vectors, sizeof(sqrt_vectors) / sizeof(sqrt_vectors[0]), 24, (const void *)&sqrtf },
	{ "hypotf", LIBM_BINARY, hypot_vectors, sizeof(hypot_vectors) / sizeof(hypot_vectors[0]), 24, (const void *)&hypotf },
	{ "powf", LIBM_BINARY, pow_vectors, sizeof(pow_vectors) / sizeof(pow_vectors[0]), 24, (const void *)&powf },
	{ "erff", LIBM_UNARY, erf_vectors, sizeof(erf_vectors) / sizeof(erf_vectors[0]), 24, (const void *)&erff },
	{ "erfcf", LIBM_UNARY, erfc_vectors, sizeof(erfc_vectors) / sizeof(erfc_vectors[0]), 24, (const void *)&erfcf },
	{ "lgammaf", LIBM_UNARY, lgamma_vectors, sizeof(lgamma_vectors) / sizeof(lgamma_vectors[0]), 24, (const void *)&lgammaf },
	{ "tgammaf", LIBM_UNARY, tgamma_vectors, sizeof(tgamma_vectors) / sizeof(tgamma_vectors[0]), 24, (const void *)&tgammaf },
	{ "ceilf", LIBM_UNARY, ceil_vectors, sizeof(ceil_vectors) / sizeof(ceil_vectors[0]), 24, (const void *)&ceilf },
	{ "floorf", LIBM_UNARY, floor_vectors, sizeof(floor_vectors) / sizeof(floor_vectors[0]), 24, (const void *)&floorf },
	{ "truncf", LIBM_UNARY, trunc_vectors, sizeof(trunc_vectors) / sizeof(trunc_vectors[0]), 24, (const void *)&truncf },
	{ "roundf", LIBM_UNARY, round_vectors, sizeof(round_vectors) / sizeof(round_vectors[0]), 24, (const void *)&roundf },
	{ "rintf", LIBM_UNARY, rint_vectors, sizeof(rint_vectors) / sizeof(rint_vectors[0]), 24, (const void *)&rintf },
	{ "nearbyintf", LIBM_UNARY, nearbyint_vectors, sizeof(nearbyint_vectors) / sizeof(nearbyint_vectors[0]), 24, (const void *)&nearbyintf },
	{ "fabsf", LIBM_UNARY, fabs_vectors, sizeof(fabs_vectors) / sizeof(fabs_vectors[0]), 24, (const void *)&fabsf },
	{ "fmodf", LIBM_BINARY, fmod_vectors, sizeof(fmod_vectors) / sizeof(fmod_vectors[0]), 24, (const void *)&fmodf },
	{ "remainderf", LIBM_BINARY, remainder_vectors, sizeof(remainder_vectors) / sizeof(remainder_vectors[0]), 24, (const void *)&remainderf },
	{ "remquof", LIBM_REMQUO, remquo_vectors, sizeof(remquo_vectors) / sizeof(remquo_vectors[0]), 24, (const void *)&remquof },
	{ "copysignf", LIBM_BINARY, copysign_vectors, sizeof(copysign_vectors) / sizeof(copysign_vectors[0]), 24, (const void *)&copysignf },
	{ "fdimf", LIBM_BINARY, fdim_vectors, sizeof(fdim_vectors) / sizeof(fdim_vectors[0]), 24, (const void *)&fdimf },
	{ "fmaxf", LIBM_BINARY, fmax_vectors, sizeof(fmax_vectors) / sizeof(fmax_vectors[0]), 24, (const void *)&fmaxf },
	{ "fminf", LIBM_BINARY, fmin_vectors, sizeof(fmin_vectors) / sizeof(fmin_vectors[0]), 24, (const void *)&fminf },
	{ "fmaf", LIBM_TERNARY, fma_vectors, sizeof(fma_vectors) / sizeof(fma_vectors[0]), 24, (const void *)&fmaf },
	{ "ldexpf", LIBM_SCALE, ldexp_vectors, sizeof(ldexp_vectors) / sizeof(ldexp_vectors[0]), 24, (const void *)&ldexpf },
	{ "scalbnf", LIBM_SCALE, scalbn_vectors, sizeof(scalbn_vectors) / sizeof(scalbn_vectors[0]), 24, (const void *)&scalbnf },
	{ "scalblnf", LIBM_SCALBLN, scalbln_vectors, sizeof(scalbln_vectors) / sizeof(scalbln_vectors[0]), 24, (const void *)&scalblnf },
	{ "lrintf", LIBM_TO_LONG, lrint_vectors, sizeof(lrint_vectors) / sizeof(lrint_vectors[0]), 24, (const void *)&lrintf },
	{ "llrintf", LIBM_TO_LLONG, llrint_vectors, sizeof(llrint_vectors) / sizeof(llrint_vectors[0]), 24, (const void *)&llrintf },
	{ "lroundf", LIBM_TO_LONG, lround_vectors, sizeof(lround_vectors) / sizeof(lround_vectors[0]), 24, (const void *)&lroundf },
	{ "llroundf", LIBM_TO_LLONG, llround_vectors, sizeof(llround_vectors) / sizeof(llround_vectors[0]), 24, (const void *)&llroundf },
	{ "ilogbf", LIBM_ILOGB, ilogb_vectors, sizeof(ilogb_vectors) / sizeof(ilogb_vectors[0]), 24, (const void *)&ilogbf },
	{ "logbf", LIBM_UNARY, logb_vectors, sizeof(logb_vectors) / sizeof(logb_vectors[0]), 24, (const void *)&logbf },
	{ "frexpf", LIBM_FREXP, frexp_vectors, sizeof(frexp_vectors) / sizeof(frexp_vectors[0]), 24, (const void *)&frexpf },
	{ "modff", LIBM_MODF, modf_vectors, sizeof(modf_vectors) / sizeof(modf_vectors[0]), 24, (const void *)&modff },
	{ "nextafterf", LIBM_NEXTAFTER, nextafter_vectors, sizeof(nextafter_vectors) / sizeof(nextafter_vectors[0]), 24, (const void *)&nextafterf },
	{ "nexttowardf", LIBM_NEXTTOWARD, nexttoward_vectors, sizeof(nexttoward_vectors) / sizeof(nexttoward_vectors[0]), 24, (const void *)&nexttowardf },
};

const size_t libm_float_function_count = sizeof(libm_float_functions) / sizeof(libm_float_functions[0]);
