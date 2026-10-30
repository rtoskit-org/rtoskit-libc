/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * vectors_d.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* The double vectors bound to the plain functions */
#include <math.h>

#include "libm_vectors.h"

#include "vectors-double.inc"

const struct libm_function libm_double_functions[] = {
	{ "acos", LIBM_UNARY, acos_vectors, sizeof(acos_vectors) / sizeof(acos_vectors[0]), 53, (const void *)&acos },
	{ "asin", LIBM_UNARY, asin_vectors, sizeof(asin_vectors) / sizeof(asin_vectors[0]), 53, (const void *)&asin },
	{ "atan", LIBM_UNARY, atan_vectors, sizeof(atan_vectors) / sizeof(atan_vectors[0]), 53, (const void *)&atan },
	{ "atan2", LIBM_BINARY, atan2_vectors, sizeof(atan2_vectors) / sizeof(atan2_vectors[0]), 53, (const void *)&atan2 },
	{ "cos", LIBM_UNARY, cos_vectors, sizeof(cos_vectors) / sizeof(cos_vectors[0]), 53, (const void *)&cos },
	{ "sin", LIBM_UNARY, sin_vectors, sizeof(sin_vectors) / sizeof(sin_vectors[0]), 53, (const void *)&sin },
	{ "tan", LIBM_UNARY, tan_vectors, sizeof(tan_vectors) / sizeof(tan_vectors[0]), 53, (const void *)&tan },
	{ "acosh", LIBM_UNARY, acosh_vectors, sizeof(acosh_vectors) / sizeof(acosh_vectors[0]), 53, (const void *)&acosh },
	{ "asinh", LIBM_UNARY, asinh_vectors, sizeof(asinh_vectors) / sizeof(asinh_vectors[0]), 53, (const void *)&asinh },
	{ "atanh", LIBM_UNARY, atanh_vectors, sizeof(atanh_vectors) / sizeof(atanh_vectors[0]), 53, (const void *)&atanh },
	{ "cosh", LIBM_UNARY, cosh_vectors, sizeof(cosh_vectors) / sizeof(cosh_vectors[0]), 53, (const void *)&cosh },
	{ "sinh", LIBM_UNARY, sinh_vectors, sizeof(sinh_vectors) / sizeof(sinh_vectors[0]), 53, (const void *)&sinh },
	{ "tanh", LIBM_UNARY, tanh_vectors, sizeof(tanh_vectors) / sizeof(tanh_vectors[0]), 53, (const void *)&tanh },
	{ "exp", LIBM_UNARY, exp_vectors, sizeof(exp_vectors) / sizeof(exp_vectors[0]), 53, (const void *)&exp },
	{ "exp2", LIBM_UNARY, exp2_vectors, sizeof(exp2_vectors) / sizeof(exp2_vectors[0]), 53, (const void *)&exp2 },
	{ "exp10", LIBM_UNARY, exp10_vectors, sizeof(exp10_vectors) / sizeof(exp10_vectors[0]), 53, (const void *)&exp10 },
	{ "expm1", LIBM_UNARY, expm1_vectors, sizeof(expm1_vectors) / sizeof(expm1_vectors[0]), 53, (const void *)&expm1 },
	{ "log", LIBM_UNARY, log_vectors, sizeof(log_vectors) / sizeof(log_vectors[0]), 53, (const void *)&log },
	{ "log2", LIBM_UNARY, log2_vectors, sizeof(log2_vectors) / sizeof(log2_vectors[0]), 53, (const void *)&log2 },
	{ "log10", LIBM_UNARY, log10_vectors, sizeof(log10_vectors) / sizeof(log10_vectors[0]), 53, (const void *)&log10 },
	{ "log1p", LIBM_UNARY, log1p_vectors, sizeof(log1p_vectors) / sizeof(log1p_vectors[0]), 53, (const void *)&log1p },
	{ "cbrt", LIBM_UNARY, cbrt_vectors, sizeof(cbrt_vectors) / sizeof(cbrt_vectors[0]), 53, (const void *)&cbrt },
	{ "sqrt", LIBM_UNARY, sqrt_vectors, sizeof(sqrt_vectors) / sizeof(sqrt_vectors[0]), 53, (const void *)&sqrt },
	{ "hypot", LIBM_BINARY, hypot_vectors, sizeof(hypot_vectors) / sizeof(hypot_vectors[0]), 53, (const void *)&hypot },
	{ "pow", LIBM_BINARY, pow_vectors, sizeof(pow_vectors) / sizeof(pow_vectors[0]), 53, (const void *)&pow },
	{ "erf", LIBM_UNARY, erf_vectors, sizeof(erf_vectors) / sizeof(erf_vectors[0]), 53, (const void *)&erf },
	{ "erfc", LIBM_UNARY, erfc_vectors, sizeof(erfc_vectors) / sizeof(erfc_vectors[0]), 53, (const void *)&erfc },
	{ "lgamma", LIBM_UNARY, lgamma_vectors, sizeof(lgamma_vectors) / sizeof(lgamma_vectors[0]), 53, (const void *)&lgamma },
	{ "tgamma", LIBM_UNARY, tgamma_vectors, sizeof(tgamma_vectors) / sizeof(tgamma_vectors[0]), 53, (const void *)&tgamma },
	{ "ceil", LIBM_UNARY, ceil_vectors, sizeof(ceil_vectors) / sizeof(ceil_vectors[0]), 53, (const void *)&ceil },
	{ "floor", LIBM_UNARY, floor_vectors, sizeof(floor_vectors) / sizeof(floor_vectors[0]), 53, (const void *)&floor },
	{ "trunc", LIBM_UNARY, trunc_vectors, sizeof(trunc_vectors) / sizeof(trunc_vectors[0]), 53, (const void *)&trunc },
	{ "round", LIBM_UNARY, round_vectors, sizeof(round_vectors) / sizeof(round_vectors[0]), 53, (const void *)&round },
	{ "rint", LIBM_UNARY, rint_vectors, sizeof(rint_vectors) / sizeof(rint_vectors[0]), 53, (const void *)&rint },
	{ "nearbyint", LIBM_UNARY, nearbyint_vectors, sizeof(nearbyint_vectors) / sizeof(nearbyint_vectors[0]), 53, (const void *)&nearbyint },
	{ "fabs", LIBM_UNARY, fabs_vectors, sizeof(fabs_vectors) / sizeof(fabs_vectors[0]), 53, (const void *)&fabs },
	{ "fmod", LIBM_BINARY, fmod_vectors, sizeof(fmod_vectors) / sizeof(fmod_vectors[0]), 53, (const void *)&fmod },
	{ "remainder", LIBM_BINARY, remainder_vectors, sizeof(remainder_vectors) / sizeof(remainder_vectors[0]), 53, (const void *)&remainder },
	{ "remquo", LIBM_REMQUO, remquo_vectors, sizeof(remquo_vectors) / sizeof(remquo_vectors[0]), 53, (const void *)&remquo },
	{ "copysign", LIBM_BINARY, copysign_vectors, sizeof(copysign_vectors) / sizeof(copysign_vectors[0]), 53, (const void *)&copysign },
	{ "fdim", LIBM_BINARY, fdim_vectors, sizeof(fdim_vectors) / sizeof(fdim_vectors[0]), 53, (const void *)&fdim },
	{ "fmax", LIBM_BINARY, fmax_vectors, sizeof(fmax_vectors) / sizeof(fmax_vectors[0]), 53, (const void *)&fmax },
	{ "fmin", LIBM_BINARY, fmin_vectors, sizeof(fmin_vectors) / sizeof(fmin_vectors[0]), 53, (const void *)&fmin },
	{ "fma", LIBM_TERNARY, fma_vectors, sizeof(fma_vectors) / sizeof(fma_vectors[0]), 53, (const void *)&fma },
	{ "ldexp", LIBM_SCALE, ldexp_vectors, sizeof(ldexp_vectors) / sizeof(ldexp_vectors[0]), 53, (const void *)&ldexp },
	{ "scalbn", LIBM_SCALE, scalbn_vectors, sizeof(scalbn_vectors) / sizeof(scalbn_vectors[0]), 53, (const void *)&scalbn },
	{ "scalbln", LIBM_SCALBLN, scalbln_vectors, sizeof(scalbln_vectors) / sizeof(scalbln_vectors[0]), 53, (const void *)&scalbln },
	{ "lrint", LIBM_TO_LONG, lrint_vectors, sizeof(lrint_vectors) / sizeof(lrint_vectors[0]), 53, (const void *)&lrint },
	{ "llrint", LIBM_TO_LLONG, llrint_vectors, sizeof(llrint_vectors) / sizeof(llrint_vectors[0]), 53, (const void *)&llrint },
	{ "lround", LIBM_TO_LONG, lround_vectors, sizeof(lround_vectors) / sizeof(lround_vectors[0]), 53, (const void *)&lround },
	{ "llround", LIBM_TO_LLONG, llround_vectors, sizeof(llround_vectors) / sizeof(llround_vectors[0]), 53, (const void *)&llround },
	{ "ilogb", LIBM_ILOGB, ilogb_vectors, sizeof(ilogb_vectors) / sizeof(ilogb_vectors[0]), 53, (const void *)&ilogb },
	{ "logb", LIBM_UNARY, logb_vectors, sizeof(logb_vectors) / sizeof(logb_vectors[0]), 53, (const void *)&logb },
	{ "frexp", LIBM_FREXP, frexp_vectors, sizeof(frexp_vectors) / sizeof(frexp_vectors[0]), 53, (const void *)&frexp },
	{ "modf", LIBM_MODF, modf_vectors, sizeof(modf_vectors) / sizeof(modf_vectors[0]), 53, (const void *)&modf },
	{ "nextafter", LIBM_NEXTAFTER, nextafter_vectors, sizeof(nextafter_vectors) / sizeof(nextafter_vectors[0]), 53, (const void *)&nextafter },
	{ "nexttoward", LIBM_NEXTTOWARD, nexttoward_vectors, sizeof(nexttoward_vectors) / sizeof(nexttoward_vectors[0]), 53, (const void *)&nexttoward },
};

const size_t libm_double_function_count = sizeof(libm_double_functions) / sizeof(libm_double_functions[0]);
