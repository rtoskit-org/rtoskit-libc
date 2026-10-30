/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * vectors_l.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* The double vectors bound to the l functions (long double is binary64, PLAN D7, so it shares the double vectors) */
#include <math.h>

#include "libm_vectors.h"

#include "vectors-double.inc"

const struct libm_function libm_long_double_functions[] = {
	{ "acosl", LIBM_UNARY, acos_vectors, sizeof(acos_vectors) / sizeof(acos_vectors[0]), 53, (const void *)&acosl },
	{ "asinl", LIBM_UNARY, asin_vectors, sizeof(asin_vectors) / sizeof(asin_vectors[0]), 53, (const void *)&asinl },
	{ "atanl", LIBM_UNARY, atan_vectors, sizeof(atan_vectors) / sizeof(atan_vectors[0]), 53, (const void *)&atanl },
	{ "atan2l", LIBM_BINARY, atan2_vectors, sizeof(atan2_vectors) / sizeof(atan2_vectors[0]), 53, (const void *)&atan2l },
	{ "cosl", LIBM_UNARY, cos_vectors, sizeof(cos_vectors) / sizeof(cos_vectors[0]), 53, (const void *)&cosl },
	{ "sinl", LIBM_UNARY, sin_vectors, sizeof(sin_vectors) / sizeof(sin_vectors[0]), 53, (const void *)&sinl },
	{ "tanl", LIBM_UNARY, tan_vectors, sizeof(tan_vectors) / sizeof(tan_vectors[0]), 53, (const void *)&tanl },
	{ "acoshl", LIBM_UNARY, acosh_vectors, sizeof(acosh_vectors) / sizeof(acosh_vectors[0]), 53, (const void *)&acoshl },
	{ "asinhl", LIBM_UNARY, asinh_vectors, sizeof(asinh_vectors) / sizeof(asinh_vectors[0]), 53, (const void *)&asinhl },
	{ "atanhl", LIBM_UNARY, atanh_vectors, sizeof(atanh_vectors) / sizeof(atanh_vectors[0]), 53, (const void *)&atanhl },
	{ "coshl", LIBM_UNARY, cosh_vectors, sizeof(cosh_vectors) / sizeof(cosh_vectors[0]), 53, (const void *)&coshl },
	{ "sinhl", LIBM_UNARY, sinh_vectors, sizeof(sinh_vectors) / sizeof(sinh_vectors[0]), 53, (const void *)&sinhl },
	{ "tanhl", LIBM_UNARY, tanh_vectors, sizeof(tanh_vectors) / sizeof(tanh_vectors[0]), 53, (const void *)&tanhl },
	{ "expl", LIBM_UNARY, exp_vectors, sizeof(exp_vectors) / sizeof(exp_vectors[0]), 53, (const void *)&expl },
	{ "exp2l", LIBM_UNARY, exp2_vectors, sizeof(exp2_vectors) / sizeof(exp2_vectors[0]), 53, (const void *)&exp2l },
	{ "exp10l", LIBM_UNARY, exp10_vectors, sizeof(exp10_vectors) / sizeof(exp10_vectors[0]), 53, (const void *)&exp10l },
	{ "expm1l", LIBM_UNARY, expm1_vectors, sizeof(expm1_vectors) / sizeof(expm1_vectors[0]), 53, (const void *)&expm1l },
	{ "logl", LIBM_UNARY, log_vectors, sizeof(log_vectors) / sizeof(log_vectors[0]), 53, (const void *)&logl },
	{ "log2l", LIBM_UNARY, log2_vectors, sizeof(log2_vectors) / sizeof(log2_vectors[0]), 53, (const void *)&log2l },
	{ "log10l", LIBM_UNARY, log10_vectors, sizeof(log10_vectors) / sizeof(log10_vectors[0]), 53, (const void *)&log10l },
	{ "log1pl", LIBM_UNARY, log1p_vectors, sizeof(log1p_vectors) / sizeof(log1p_vectors[0]), 53, (const void *)&log1pl },
	{ "cbrtl", LIBM_UNARY, cbrt_vectors, sizeof(cbrt_vectors) / sizeof(cbrt_vectors[0]), 53, (const void *)&cbrtl },
	{ "sqrtl", LIBM_UNARY, sqrt_vectors, sizeof(sqrt_vectors) / sizeof(sqrt_vectors[0]), 53, (const void *)&sqrtl },
	{ "hypotl", LIBM_BINARY, hypot_vectors, sizeof(hypot_vectors) / sizeof(hypot_vectors[0]), 53, (const void *)&hypotl },
	{ "powl", LIBM_BINARY, pow_vectors, sizeof(pow_vectors) / sizeof(pow_vectors[0]), 53, (const void *)&powl },
	{ "erfl", LIBM_UNARY, erf_vectors, sizeof(erf_vectors) / sizeof(erf_vectors[0]), 53, (const void *)&erfl },
	{ "erfcl", LIBM_UNARY, erfc_vectors, sizeof(erfc_vectors) / sizeof(erfc_vectors[0]), 53, (const void *)&erfcl },
	{ "lgammal", LIBM_UNARY, lgamma_vectors, sizeof(lgamma_vectors) / sizeof(lgamma_vectors[0]), 53, (const void *)&lgammal },
	{ "tgammal", LIBM_UNARY, tgamma_vectors, sizeof(tgamma_vectors) / sizeof(tgamma_vectors[0]), 53, (const void *)&tgammal },
	{ "ceill", LIBM_UNARY, ceil_vectors, sizeof(ceil_vectors) / sizeof(ceil_vectors[0]), 53, (const void *)&ceill },
	{ "floorl", LIBM_UNARY, floor_vectors, sizeof(floor_vectors) / sizeof(floor_vectors[0]), 53, (const void *)&floorl },
	{ "truncl", LIBM_UNARY, trunc_vectors, sizeof(trunc_vectors) / sizeof(trunc_vectors[0]), 53, (const void *)&truncl },
	{ "roundl", LIBM_UNARY, round_vectors, sizeof(round_vectors) / sizeof(round_vectors[0]), 53, (const void *)&roundl },
	{ "rintl", LIBM_UNARY, rint_vectors, sizeof(rint_vectors) / sizeof(rint_vectors[0]), 53, (const void *)&rintl },
	{ "nearbyintl", LIBM_UNARY, nearbyint_vectors, sizeof(nearbyint_vectors) / sizeof(nearbyint_vectors[0]), 53, (const void *)&nearbyintl },
	{ "fabsl", LIBM_UNARY, fabs_vectors, sizeof(fabs_vectors) / sizeof(fabs_vectors[0]), 53, (const void *)&fabsl },
	{ "fmodl", LIBM_BINARY, fmod_vectors, sizeof(fmod_vectors) / sizeof(fmod_vectors[0]), 53, (const void *)&fmodl },
	{ "remainderl", LIBM_BINARY, remainder_vectors, sizeof(remainder_vectors) / sizeof(remainder_vectors[0]), 53, (const void *)&remainderl },
	{ "remquol", LIBM_REMQUO, remquo_vectors, sizeof(remquo_vectors) / sizeof(remquo_vectors[0]), 53, (const void *)&remquol },
	{ "copysignl", LIBM_BINARY, copysign_vectors, sizeof(copysign_vectors) / sizeof(copysign_vectors[0]), 53, (const void *)&copysignl },
	{ "fdiml", LIBM_BINARY, fdim_vectors, sizeof(fdim_vectors) / sizeof(fdim_vectors[0]), 53, (const void *)&fdiml },
	{ "fmaxl", LIBM_BINARY, fmax_vectors, sizeof(fmax_vectors) / sizeof(fmax_vectors[0]), 53, (const void *)&fmaxl },
	{ "fminl", LIBM_BINARY, fmin_vectors, sizeof(fmin_vectors) / sizeof(fmin_vectors[0]), 53, (const void *)&fminl },
	{ "fmal", LIBM_TERNARY, fma_vectors, sizeof(fma_vectors) / sizeof(fma_vectors[0]), 53, (const void *)&fmal },
	{ "ldexpl", LIBM_SCALE, ldexp_vectors, sizeof(ldexp_vectors) / sizeof(ldexp_vectors[0]), 53, (const void *)&ldexpl },
	{ "scalbnl", LIBM_SCALE, scalbn_vectors, sizeof(scalbn_vectors) / sizeof(scalbn_vectors[0]), 53, (const void *)&scalbnl },
	{ "scalblnl", LIBM_SCALBLN, scalbln_vectors, sizeof(scalbln_vectors) / sizeof(scalbln_vectors[0]), 53, (const void *)&scalblnl },
	{ "lrintl", LIBM_TO_LONG, lrint_vectors, sizeof(lrint_vectors) / sizeof(lrint_vectors[0]), 53, (const void *)&lrintl },
	{ "llrintl", LIBM_TO_LLONG, llrint_vectors, sizeof(llrint_vectors) / sizeof(llrint_vectors[0]), 53, (const void *)&llrintl },
	{ "lroundl", LIBM_TO_LONG, lround_vectors, sizeof(lround_vectors) / sizeof(lround_vectors[0]), 53, (const void *)&lroundl },
	{ "llroundl", LIBM_TO_LLONG, llround_vectors, sizeof(llround_vectors) / sizeof(llround_vectors[0]), 53, (const void *)&llroundl },
	{ "ilogbl", LIBM_ILOGB, ilogb_vectors, sizeof(ilogb_vectors) / sizeof(ilogb_vectors[0]), 53, (const void *)&ilogbl },
	{ "logbl", LIBM_UNARY, logb_vectors, sizeof(logb_vectors) / sizeof(logb_vectors[0]), 53, (const void *)&logbl },
	{ "frexpl", LIBM_FREXP, frexp_vectors, sizeof(frexp_vectors) / sizeof(frexp_vectors[0]), 53, (const void *)&frexpl },
	{ "modfl", LIBM_MODF, modf_vectors, sizeof(modf_vectors) / sizeof(modf_vectors[0]), 53, (const void *)&modfl },
	{ "nextafterl", LIBM_NEXTAFTER, nextafter_vectors, sizeof(nextafter_vectors) / sizeof(nextafter_vectors[0]), 53, (const void *)&nextafterl },
	{ "nexttowardl", LIBM_NEXTTOWARD, nexttoward_vectors, sizeof(nexttoward_vectors) / sizeof(nexttoward_vectors[0]), 53, (const void *)&nexttowardl },
};

const size_t libm_long_double_function_count = sizeof(libm_long_double_functions) / sizeof(libm_long_double_functions[0]);
