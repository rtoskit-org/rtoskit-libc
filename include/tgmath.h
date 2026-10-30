/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tgmath.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _TGMATH_H
#define _TGMATH_H

/* C++ takes the type-generic forms from <cmath> and <complex> (libstdc++'s <tgmath.h>) */
#ifndef __cplusplus
/*
 * C23 7.27 through _Generic (docs/research/libm-complex.md). An argument's
 * type selects the function: integers count as double (7.27p7), several
 * generic arguments combine by the usual arithmetic conversions (their
 * __TG_TYPE values are added in an unevaluated _Generic controlling
 * expression), and a complex argument selects the complex function.
 * Decimal floating types are not supported (PLAN §1).
 */

#include <complex.h>
#include <math.h>

#define __STDC_VERSION_TGMATH_H__ 202311L

#define __TG_TYPE(__x) _Generic((__x), float: (float)0, long double: (long double)0, float _Complex: (float _Complex)0, double _Complex: (double _Complex)0, long double _Complex: (long double _Complex)0, default: (double)0)
#define __TG_REAL(__t, __f) _Generic((__t), float: __f##f, long double: __f##l, default: __f)
#define __TG_COMPLEX(__t, __f, __c) _Generic((__t), float: __f##f, long double: __f##l, float _Complex: __c##f, double _Complex: __c, long double _Complex: __c##l, default: __f)
#define __TG_COMPLEX_ONLY(__t, __c) _Generic((__t), float: __c##f, long double: __c##l, float _Complex: __c##f, long double _Complex: __c##l, default: __c)

#define acos(__x) __TG_COMPLEX(__TG_TYPE(__x), acos, cacos)(__x)
#define asin(__x) __TG_COMPLEX(__TG_TYPE(__x), asin, casin)(__x)
#define atan(__x) __TG_COMPLEX(__TG_TYPE(__x), atan, catan)(__x)
#define acosh(__x) __TG_COMPLEX(__TG_TYPE(__x), acosh, cacosh)(__x)
#define asinh(__x) __TG_COMPLEX(__TG_TYPE(__x), asinh, casinh)(__x)
#define atanh(__x) __TG_COMPLEX(__TG_TYPE(__x), atanh, catanh)(__x)
#define cos(__x) __TG_COMPLEX(__TG_TYPE(__x), cos, ccos)(__x)
#define sin(__x) __TG_COMPLEX(__TG_TYPE(__x), sin, csin)(__x)
#define tan(__x) __TG_COMPLEX(__TG_TYPE(__x), tan, ctan)(__x)
#define cosh(__x) __TG_COMPLEX(__TG_TYPE(__x), cosh, ccosh)(__x)
#define sinh(__x) __TG_COMPLEX(__TG_TYPE(__x), sinh, csinh)(__x)
#define tanh(__x) __TG_COMPLEX(__TG_TYPE(__x), tanh, ctanh)(__x)
#define exp(__x) __TG_COMPLEX(__TG_TYPE(__x), exp, cexp)(__x)
#define log(__x) __TG_COMPLEX(__TG_TYPE(__x), log, clog)(__x)
#define sqrt(__x) __TG_COMPLEX(__TG_TYPE(__x), sqrt, csqrt)(__x)
#define pow(__x, __y) __TG_COMPLEX(__TG_TYPE(__x) + __TG_TYPE(__y), pow, cpow)(__x, __y)
#define fabs(__x) __TG_COMPLEX(__TG_TYPE(__x), fabs, cabs)(__x)
#define acospi(__x) __TG_REAL(__TG_TYPE(__x), acospi)(__x)
#define asinpi(__x) __TG_REAL(__TG_TYPE(__x), asinpi)(__x)
#define atan2(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), atan2)(__x, __y)
#define atan2pi(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), atan2pi)(__x, __y)
#define atanpi(__x) __TG_REAL(__TG_TYPE(__x), atanpi)(__x)
#define cbrt(__x) __TG_REAL(__TG_TYPE(__x), cbrt)(__x)
#define ceil(__x) __TG_REAL(__TG_TYPE(__x), ceil)(__x)
#define compoundn(__x, __y) __TG_REAL(__TG_TYPE(__x), compoundn)(__x, __y)
#define copysign(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), copysign)(__x, __y)
#define cospi(__x) __TG_REAL(__TG_TYPE(__x), cospi)(__x)
#define erf(__x) __TG_REAL(__TG_TYPE(__x), erf)(__x)
#define erfc(__x) __TG_REAL(__TG_TYPE(__x), erfc)(__x)
#define exp10(__x) __TG_REAL(__TG_TYPE(__x), exp10)(__x)
#define exp10m1(__x) __TG_REAL(__TG_TYPE(__x), exp10m1)(__x)
#define exp2(__x) __TG_REAL(__TG_TYPE(__x), exp2)(__x)
#define exp2m1(__x) __TG_REAL(__TG_TYPE(__x), exp2m1)(__x)
#define expm1(__x) __TG_REAL(__TG_TYPE(__x), expm1)(__x)
#define fdim(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fdim)(__x, __y)
#define floor(__x) __TG_REAL(__TG_TYPE(__x), floor)(__x)
#define fma(__x, __y, __z) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y) + __TG_TYPE(__z), fma)(__x, __y, __z)
#define fmax(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmax)(__x, __y)
#define fmaximum(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmaximum)(__x, __y)
#define fmaximum_mag(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmaximum_mag)(__x, __y)
#define fmaximum_mag_num(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmaximum_mag_num)(__x, __y)
#define fmaximum_num(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmaximum_num)(__x, __y)
#define fmin(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmin)(__x, __y)
#define fminimum(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fminimum)(__x, __y)
#define fminimum_mag(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fminimum_mag)(__x, __y)
#define fminimum_mag_num(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fminimum_mag_num)(__x, __y)
#define fminimum_num(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fminimum_num)(__x, __y)
#define fmod(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), fmod)(__x, __y)
#define frexp(__x, __y) __TG_REAL(__TG_TYPE(__x), frexp)(__x, __y)
#define fromfp(__x, __y, __z) __TG_REAL(__TG_TYPE(__x), fromfp)(__x, __y, __z)
#define fromfpx(__x, __y, __z) __TG_REAL(__TG_TYPE(__x), fromfpx)(__x, __y, __z)
#define hypot(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), hypot)(__x, __y)
#define ilogb(__x) __TG_REAL(__TG_TYPE(__x), ilogb)(__x)
#define ldexp(__x, __y) __TG_REAL(__TG_TYPE(__x), ldexp)(__x, __y)
#define lgamma(__x) __TG_REAL(__TG_TYPE(__x), lgamma)(__x)
#define llogb(__x) __TG_REAL(__TG_TYPE(__x), llogb)(__x)
#define llrint(__x) __TG_REAL(__TG_TYPE(__x), llrint)(__x)
#define llround(__x) __TG_REAL(__TG_TYPE(__x), llround)(__x)
#define log10(__x) __TG_REAL(__TG_TYPE(__x), log10)(__x)
#define log10p1(__x) __TG_REAL(__TG_TYPE(__x), log10p1)(__x)
#define log1p(__x) __TG_REAL(__TG_TYPE(__x), log1p)(__x)
#define log2(__x) __TG_REAL(__TG_TYPE(__x), log2)(__x)
#define log2p1(__x) __TG_REAL(__TG_TYPE(__x), log2p1)(__x)
#define logb(__x) __TG_REAL(__TG_TYPE(__x), logb)(__x)
#define logp1(__x) __TG_REAL(__TG_TYPE(__x), logp1)(__x)
#define lrint(__x) __TG_REAL(__TG_TYPE(__x), lrint)(__x)
#define lround(__x) __TG_REAL(__TG_TYPE(__x), lround)(__x)
#define nearbyint(__x) __TG_REAL(__TG_TYPE(__x), nearbyint)(__x)
#define nextafter(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), nextafter)(__x, __y)
#define nextdown(__x) __TG_REAL(__TG_TYPE(__x), nextdown)(__x)
#define nexttoward(__x, __y) __TG_REAL(__TG_TYPE(__x), nexttoward)(__x, __y)
#define nextup(__x) __TG_REAL(__TG_TYPE(__x), nextup)(__x)
#define pown(__x, __y) __TG_REAL(__TG_TYPE(__x), pown)(__x, __y)
#define powr(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), powr)(__x, __y)
#define remainder(__x, __y) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), remainder)(__x, __y)
#define remquo(__x, __y, __z) __TG_REAL(__TG_TYPE(__x) + __TG_TYPE(__y), remquo)(__x, __y, __z)
#define rint(__x) __TG_REAL(__TG_TYPE(__x), rint)(__x)
#define rootn(__x, __y) __TG_REAL(__TG_TYPE(__x), rootn)(__x, __y)
#define round(__x) __TG_REAL(__TG_TYPE(__x), round)(__x)
#define roundeven(__x) __TG_REAL(__TG_TYPE(__x), roundeven)(__x)
#define rsqrt(__x) __TG_REAL(__TG_TYPE(__x), rsqrt)(__x)
#define scalbln(__x, __y) __TG_REAL(__TG_TYPE(__x), scalbln)(__x, __y)
#define scalbn(__x, __y) __TG_REAL(__TG_TYPE(__x), scalbn)(__x, __y)
#define sinpi(__x) __TG_REAL(__TG_TYPE(__x), sinpi)(__x)
#define tanpi(__x) __TG_REAL(__TG_TYPE(__x), tanpi)(__x)
#define tgamma(__x) __TG_REAL(__TG_TYPE(__x), tgamma)(__x)
#define trunc(__x) __TG_REAL(__TG_TYPE(__x), trunc)(__x)
#define ufromfp(__x, __y, __z) __TG_REAL(__TG_TYPE(__x), ufromfp)(__x, __y, __z)
#define ufromfpx(__x, __y, __z) __TG_REAL(__TG_TYPE(__x), ufromfpx)(__x, __y, __z)
#define carg(__x) __TG_COMPLEX_ONLY(__TG_TYPE(__x), carg)(__x)
#define cimag(__x) __TG_COMPLEX_ONLY(__TG_TYPE(__x), cimag)(__x)
#define conj(__x) __TG_COMPLEX_ONLY(__TG_TYPE(__x), conj)(__x)
#define cproj(__x) __TG_COMPLEX_ONLY(__TG_TYPE(__x), cproj)(__x)
#define creal(__x) __TG_COMPLEX_ONLY(__TG_TYPE(__x), creal)(__x)
#define fadd(__x, __y) _Generic(__TG_TYPE(__x) + __TG_TYPE(__y), long double: faddl, default: fadd)(__x, __y)
#define dadd(__x, __y) daddl(__x, __y)
#define fsub(__x, __y) _Generic(__TG_TYPE(__x) + __TG_TYPE(__y), long double: fsubl, default: fsub)(__x, __y)
#define dsub(__x, __y) dsubl(__x, __y)
#define fmul(__x, __y) _Generic(__TG_TYPE(__x) + __TG_TYPE(__y), long double: fmull, default: fmul)(__x, __y)
#define dmul(__x, __y) dmull(__x, __y)
#define fdiv(__x, __y) _Generic(__TG_TYPE(__x) + __TG_TYPE(__y), long double: fdivl, default: fdiv)(__x, __y)
#define ddiv(__x, __y) ddivl(__x, __y)
#define ffma(__x, __y, __z) _Generic(__TG_TYPE(__x) + __TG_TYPE(__y) + __TG_TYPE(__z), long double: ffmal, default: ffma)(__x, __y, __z)
#define dfma(__x, __y, __z) dfmal(__x, __y, __z)
#define fsqrt(__x) _Generic(__TG_TYPE(__x), long double: fsqrtl, default: fsqrt)(__x)
#define dsqrt(__x) dsqrtl(__x)

#endif /* __cplusplus */

#endif /* _TGMATH_H */
