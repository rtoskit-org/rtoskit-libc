/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * annexb.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Generated from the C23 Annex B (N3220, docs/standard/annex-b.pdf) by
 * tools/scripts/annexb-audit (PLAN D50); regenerate rather than edit. The
 * test passes when it compiles: every identifier of the in-scope headers
 * exists, every function has the Annex B type.
 */

#define __STDC_WANT_IEC_60559_EXT__
#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbit.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <tgmath.h>
#include <threads.h>
#include <time.h>

#include <stddef.h>

/* Every macro, type and constant: defined as a macro, or else usable in sizeof */
#ifndef __STDC_VERSION_ASSERT_H__
static_assert(sizeof(__STDC_VERSION_ASSERT_H__) > 0, "<assert.h> __STDC_VERSION_ASSERT_H__");
#endif
#ifndef __STDC_VERSION_COMPLEX_H__
static_assert(sizeof(__STDC_VERSION_COMPLEX_H__) > 0, "<complex.h> __STDC_VERSION_COMPLEX_H__");
#endif
#ifndef complex
static_assert(sizeof(complex) > 0, "<complex.h> complex");
#endif
#ifndef I
static_assert(sizeof(I) > 0, "<complex.h> I");
#endif
#ifndef _Complex_I
static_assert(sizeof(_Complex_I) > 0, "<complex.h> _Complex_I");
#endif
#ifndef EDOM
static_assert(sizeof(EDOM) > 0, "<errno.h> EDOM");
#endif
#ifndef EILSEQ
static_assert(sizeof(EILSEQ) > 0, "<errno.h> EILSEQ");
#endif
#ifndef ERANGE
static_assert(sizeof(ERANGE) > 0, "<errno.h> ERANGE");
#endif
#ifndef errno
static_assert(sizeof(errno) > 0, "<errno.h> errno");
#endif
#ifndef fenv_t
static_assert(sizeof(fenv_t) > 0, "<fenv.h> fenv_t");
#endif
#ifndef fexcept_t
static_assert(sizeof(fexcept_t) > 0, "<fenv.h> fexcept_t");
#endif
#ifndef femode_t
static_assert(sizeof(femode_t) > 0, "<fenv.h> femode_t");
#endif
#ifndef FE_DFL_MODE
static_assert(sizeof(FE_DFL_MODE) > 0, "<fenv.h> FE_DFL_MODE");
#endif
#ifndef FE_ALL_EXCEPT
static_assert(sizeof(FE_ALL_EXCEPT) > 0, "<fenv.h> FE_ALL_EXCEPT");
#endif
#ifndef FE_DFL_ENV
static_assert(sizeof(FE_DFL_ENV) > 0, "<fenv.h> FE_DFL_ENV");
#endif
#ifndef __STDC_VERSION_FENV_H__
static_assert(sizeof(__STDC_VERSION_FENV_H__) > 0, "<fenv.h> __STDC_VERSION_FENV_H__");
#endif
#ifndef FE_TONEAREST
static_assert(sizeof(FE_TONEAREST) > 0, "<fenv.h> FE_TONEAREST");
#endif
#ifndef __STDC_VERSION_FLOAT_H__
static_assert(sizeof(__STDC_VERSION_FLOAT_H__) > 0, "<float.h> __STDC_VERSION_FLOAT_H__");
#endif
#ifndef FLT_MAX_EXP
static_assert(sizeof(FLT_MAX_EXP) > 0, "<float.h> FLT_MAX_EXP");
#endif
#ifndef FLT_ROUNDS
static_assert(sizeof(FLT_ROUNDS) > 0, "<float.h> FLT_ROUNDS");
#endif
#ifndef FLT_IS_IEC_60559
static_assert(sizeof(FLT_IS_IEC_60559) > 0, "<float.h> FLT_IS_IEC_60559");
#endif
#ifndef DBL_MAX_EXP
static_assert(sizeof(DBL_MAX_EXP) > 0, "<float.h> DBL_MAX_EXP");
#endif
#ifndef FLT_EVAL_METHOD
static_assert(sizeof(FLT_EVAL_METHOD) > 0, "<float.h> FLT_EVAL_METHOD");
#endif
#ifndef DBL_IS_IEC_60559
static_assert(sizeof(DBL_IS_IEC_60559) > 0, "<float.h> DBL_IS_IEC_60559");
#endif
#ifndef LDBL_MAX_EXP
static_assert(sizeof(LDBL_MAX_EXP) > 0, "<float.h> LDBL_MAX_EXP");
#endif
#ifndef FLT_HAS_SUBNORM
static_assert(sizeof(FLT_HAS_SUBNORM) > 0, "<float.h> FLT_HAS_SUBNORM");
#endif
#ifndef LDBL_IS_IEC_60559
static_assert(sizeof(LDBL_IS_IEC_60559) > 0, "<float.h> LDBL_IS_IEC_60559");
#endif
#ifndef FLT_MAX_10_EXP
static_assert(sizeof(FLT_MAX_10_EXP) > 0, "<float.h> FLT_MAX_10_EXP");
#endif
#ifndef DBL_HAS_SUBNORM
static_assert(sizeof(DBL_HAS_SUBNORM) > 0, "<float.h> DBL_HAS_SUBNORM");
#endif
#ifndef FLT_DIG
static_assert(sizeof(FLT_DIG) > 0, "<float.h> FLT_DIG");
#endif
#ifndef DBL_MAX_10_EXP
static_assert(sizeof(DBL_MAX_10_EXP) > 0, "<float.h> DBL_MAX_10_EXP");
#endif
#ifndef LDBL_HAS_SUBNORM
static_assert(sizeof(LDBL_HAS_SUBNORM) > 0, "<float.h> LDBL_HAS_SUBNORM");
#endif
#ifndef DBL_DIG
static_assert(sizeof(DBL_DIG) > 0, "<float.h> DBL_DIG");
#endif
#ifndef LDBL_MAX_10_EXP
static_assert(sizeof(LDBL_MAX_10_EXP) > 0, "<float.h> LDBL_MAX_10_EXP");
#endif
#ifndef FLT_RADIX
static_assert(sizeof(FLT_RADIX) > 0, "<float.h> FLT_RADIX");
#endif
#ifndef LDBL_DIG
static_assert(sizeof(LDBL_DIG) > 0, "<float.h> LDBL_DIG");
#endif
#ifndef FLT_MAX
static_assert(sizeof(FLT_MAX) > 0, "<float.h> FLT_MAX");
#endif
#ifndef FLT_MANT_DIG
static_assert(sizeof(FLT_MANT_DIG) > 0, "<float.h> FLT_MANT_DIG");
#endif
#ifndef FLT_MIN_EXP
static_assert(sizeof(FLT_MIN_EXP) > 0, "<float.h> FLT_MIN_EXP");
#endif
#ifndef DBL_MAX
static_assert(sizeof(DBL_MAX) > 0, "<float.h> DBL_MAX");
#endif
#ifndef DBL_MANT_DIG
static_assert(sizeof(DBL_MANT_DIG) > 0, "<float.h> DBL_MANT_DIG");
#endif
#ifndef DBL_MIN_EXP
static_assert(sizeof(DBL_MIN_EXP) > 0, "<float.h> DBL_MIN_EXP");
#endif
#ifndef LDBL_MAX
static_assert(sizeof(LDBL_MAX) > 0, "<float.h> LDBL_MAX");
#endif
#ifndef LDBL_MANT_DIG
static_assert(sizeof(LDBL_MANT_DIG) > 0, "<float.h> LDBL_MANT_DIG");
#endif
#ifndef LDBL_MIN_EXP
static_assert(sizeof(LDBL_MIN_EXP) > 0, "<float.h> LDBL_MIN_EXP");
#endif
#ifndef FLT_NORM_MAX
static_assert(sizeof(FLT_NORM_MAX) > 0, "<float.h> FLT_NORM_MAX");
#endif
#ifndef FLT_DECIMAL_DIG
static_assert(sizeof(FLT_DECIMAL_DIG) > 0, "<float.h> FLT_DECIMAL_DIG");
#endif
#ifndef FLT_MIN_10_EXP
static_assert(sizeof(FLT_MIN_10_EXP) > 0, "<float.h> FLT_MIN_10_EXP");
#endif
#ifndef DBL_NORM_MAX
static_assert(sizeof(DBL_NORM_MAX) > 0, "<float.h> DBL_NORM_MAX");
#endif
#ifndef DBL_DECIMAL_DIG
static_assert(sizeof(DBL_DECIMAL_DIG) > 0, "<float.h> DBL_DECIMAL_DIG");
#endif
#ifndef DBL_MIN_10_EXP
static_assert(sizeof(DBL_MIN_10_EXP) > 0, "<float.h> DBL_MIN_10_EXP");
#endif
#ifndef LDBL_NORM_MAX
static_assert(sizeof(LDBL_NORM_MAX) > 0, "<float.h> LDBL_NORM_MAX");
#endif
#ifndef LDBL_DECIMAL_DIG
static_assert(sizeof(LDBL_DECIMAL_DIG) > 0, "<float.h> LDBL_DECIMAL_DIG");
#endif
#ifndef LDBL_MIN_10_EXP
static_assert(sizeof(LDBL_MIN_10_EXP) > 0, "<float.h> LDBL_MIN_10_EXP");
#endif
#ifndef FLT_EPSILON
static_assert(sizeof(FLT_EPSILON) > 0, "<float.h> FLT_EPSILON");
#endif
#ifndef DBL_EPSILON
static_assert(sizeof(DBL_EPSILON) > 0, "<float.h> DBL_EPSILON");
#endif
#ifndef FLT_SNAN
static_assert(sizeof(FLT_SNAN) > 0, "<float.h> FLT_SNAN");
#endif
#ifndef LDBL_TRUE_MIN
static_assert(sizeof(LDBL_TRUE_MIN) > 0, "<float.h> LDBL_TRUE_MIN");
#endif
#ifndef LDBL_EPSILON
static_assert(sizeof(LDBL_EPSILON) > 0, "<float.h> LDBL_EPSILON");
#endif
#ifndef DBL_SNAN
static_assert(sizeof(DBL_SNAN) > 0, "<float.h> DBL_SNAN");
#endif
#ifndef INFINITY
static_assert(sizeof(INFINITY) > 0, "<float.h> INFINITY");
#endif
#ifndef FLT_MIN
static_assert(sizeof(FLT_MIN) > 0, "<float.h> FLT_MIN");
#endif
#ifndef LDBL_SNAN
static_assert(sizeof(LDBL_SNAN) > 0, "<float.h> LDBL_SNAN");
#endif
#ifndef NAN
static_assert(sizeof(NAN) > 0, "<float.h> NAN");
#endif
#ifndef DBL_MIN
static_assert(sizeof(DBL_MIN) > 0, "<float.h> DBL_MIN");
#endif
#ifndef FLT_TRUE_MIN
static_assert(sizeof(FLT_TRUE_MIN) > 0, "<float.h> FLT_TRUE_MIN");
#endif
#ifndef LDBL_MIN
static_assert(sizeof(LDBL_MIN) > 0, "<float.h> LDBL_MIN");
#endif
#ifndef DBL_TRUE_MIN
static_assert(sizeof(DBL_TRUE_MIN) > 0, "<float.h> DBL_TRUE_MIN");
#endif
#ifndef CR_DECIMAL_DIG
static_assert(sizeof(CR_DECIMAL_DIG) > 0, "<float.h> CR_DECIMAL_DIG");
#endif
#ifndef __STDC_VERSION_INTTYPES_H__
static_assert(sizeof(__STDC_VERSION_INTTYPES_H__) > 0, "<inttypes.h> __STDC_VERSION_INTTYPES_H__");
#endif
#ifndef imaxdiv_t
static_assert(sizeof(imaxdiv_t) > 0, "<inttypes.h> imaxdiv_t");
#endif
#ifndef PRIb8
static_assert(sizeof(PRIb8) > 0, "<inttypes.h> PRIb8");
#endif
#ifndef PRIb16
static_assert(sizeof(PRIb16) > 0, "<inttypes.h> PRIb16");
#endif
#ifndef PRIb32
static_assert(sizeof(PRIb32) > 0, "<inttypes.h> PRIb32");
#endif
#ifndef PRIb64
static_assert(sizeof(PRIb64) > 0, "<inttypes.h> PRIb64");
#endif
#ifndef PRIbLEAST8
static_assert(sizeof(PRIbLEAST8) > 0, "<inttypes.h> PRIbLEAST8");
#endif
#ifndef PRIbLEAST16
static_assert(sizeof(PRIbLEAST16) > 0, "<inttypes.h> PRIbLEAST16");
#endif
#ifndef PRIbLEAST32
static_assert(sizeof(PRIbLEAST32) > 0, "<inttypes.h> PRIbLEAST32");
#endif
#ifndef PRIbLEAST64
static_assert(sizeof(PRIbLEAST64) > 0, "<inttypes.h> PRIbLEAST64");
#endif
#ifndef PRIbFAST8
static_assert(sizeof(PRIbFAST8) > 0, "<inttypes.h> PRIbFAST8");
#endif
#ifndef PRIbFAST16
static_assert(sizeof(PRIbFAST16) > 0, "<inttypes.h> PRIbFAST16");
#endif
#ifndef PRIbFAST32
static_assert(sizeof(PRIbFAST32) > 0, "<inttypes.h> PRIbFAST32");
#endif
#ifndef PRIbFAST64
static_assert(sizeof(PRIbFAST64) > 0, "<inttypes.h> PRIbFAST64");
#endif
#ifndef PRIbMAX
static_assert(sizeof(PRIbMAX) > 0, "<inttypes.h> PRIbMAX");
#endif
#ifndef PRIbPTR
static_assert(sizeof(PRIbPTR) > 0, "<inttypes.h> PRIbPTR");
#endif
#ifndef PRIB8
static_assert(sizeof(PRIB8) > 0, "<inttypes.h> PRIB8");
#endif
#ifndef PRIB16
static_assert(sizeof(PRIB16) > 0, "<inttypes.h> PRIB16");
#endif
#ifndef PRIB32
static_assert(sizeof(PRIB32) > 0, "<inttypes.h> PRIB32");
#endif
#ifndef PRIB64
static_assert(sizeof(PRIB64) > 0, "<inttypes.h> PRIB64");
#endif
#ifndef PRIBLEAST8
static_assert(sizeof(PRIBLEAST8) > 0, "<inttypes.h> PRIBLEAST8");
#endif
#ifndef PRIBLEAST16
static_assert(sizeof(PRIBLEAST16) > 0, "<inttypes.h> PRIBLEAST16");
#endif
#ifndef PRIBLEAST32
static_assert(sizeof(PRIBLEAST32) > 0, "<inttypes.h> PRIBLEAST32");
#endif
#ifndef PRIBLEAST64
static_assert(sizeof(PRIBLEAST64) > 0, "<inttypes.h> PRIBLEAST64");
#endif
#ifndef PRIBFAST8
static_assert(sizeof(PRIBFAST8) > 0, "<inttypes.h> PRIBFAST8");
#endif
#ifndef PRIBFAST16
static_assert(sizeof(PRIBFAST16) > 0, "<inttypes.h> PRIBFAST16");
#endif
#ifndef PRIBFAST32
static_assert(sizeof(PRIBFAST32) > 0, "<inttypes.h> PRIBFAST32");
#endif
#ifndef PRIBFAST64
static_assert(sizeof(PRIBFAST64) > 0, "<inttypes.h> PRIBFAST64");
#endif
#ifndef PRIBMAX
static_assert(sizeof(PRIBMAX) > 0, "<inttypes.h> PRIBMAX");
#endif
#ifndef PRIBPTR
static_assert(sizeof(PRIBPTR) > 0, "<inttypes.h> PRIBPTR");
#endif
#ifndef PRId8
static_assert(sizeof(PRId8) > 0, "<inttypes.h> PRId8");
#endif
#ifndef PRId16
static_assert(sizeof(PRId16) > 0, "<inttypes.h> PRId16");
#endif
#ifndef PRId32
static_assert(sizeof(PRId32) > 0, "<inttypes.h> PRId32");
#endif
#ifndef PRId64
static_assert(sizeof(PRId64) > 0, "<inttypes.h> PRId64");
#endif
#ifndef PRIdLEAST8
static_assert(sizeof(PRIdLEAST8) > 0, "<inttypes.h> PRIdLEAST8");
#endif
#ifndef PRIdLEAST16
static_assert(sizeof(PRIdLEAST16) > 0, "<inttypes.h> PRIdLEAST16");
#endif
#ifndef PRIdLEAST32
static_assert(sizeof(PRIdLEAST32) > 0, "<inttypes.h> PRIdLEAST32");
#endif
#ifndef PRIdLEAST64
static_assert(sizeof(PRIdLEAST64) > 0, "<inttypes.h> PRIdLEAST64");
#endif
#ifndef PRIdFAST8
static_assert(sizeof(PRIdFAST8) > 0, "<inttypes.h> PRIdFAST8");
#endif
#ifndef PRIdFAST16
static_assert(sizeof(PRIdFAST16) > 0, "<inttypes.h> PRIdFAST16");
#endif
#ifndef PRIdFAST32
static_assert(sizeof(PRIdFAST32) > 0, "<inttypes.h> PRIdFAST32");
#endif
#ifndef PRIdFAST64
static_assert(sizeof(PRIdFAST64) > 0, "<inttypes.h> PRIdFAST64");
#endif
#ifndef PRIdMAX
static_assert(sizeof(PRIdMAX) > 0, "<inttypes.h> PRIdMAX");
#endif
#ifndef PRIdPTR
static_assert(sizeof(PRIdPTR) > 0, "<inttypes.h> PRIdPTR");
#endif
#ifndef PRIi8
static_assert(sizeof(PRIi8) > 0, "<inttypes.h> PRIi8");
#endif
#ifndef PRIi16
static_assert(sizeof(PRIi16) > 0, "<inttypes.h> PRIi16");
#endif
#ifndef PRIi32
static_assert(sizeof(PRIi32) > 0, "<inttypes.h> PRIi32");
#endif
#ifndef PRIi64
static_assert(sizeof(PRIi64) > 0, "<inttypes.h> PRIi64");
#endif
#ifndef PRIiLEAST8
static_assert(sizeof(PRIiLEAST8) > 0, "<inttypes.h> PRIiLEAST8");
#endif
#ifndef PRIiLEAST16
static_assert(sizeof(PRIiLEAST16) > 0, "<inttypes.h> PRIiLEAST16");
#endif
#ifndef PRIiLEAST32
static_assert(sizeof(PRIiLEAST32) > 0, "<inttypes.h> PRIiLEAST32");
#endif
#ifndef PRIiLEAST64
static_assert(sizeof(PRIiLEAST64) > 0, "<inttypes.h> PRIiLEAST64");
#endif
#ifndef PRIiFAST8
static_assert(sizeof(PRIiFAST8) > 0, "<inttypes.h> PRIiFAST8");
#endif
#ifndef PRIiFAST16
static_assert(sizeof(PRIiFAST16) > 0, "<inttypes.h> PRIiFAST16");
#endif
#ifndef PRIiFAST32
static_assert(sizeof(PRIiFAST32) > 0, "<inttypes.h> PRIiFAST32");
#endif
#ifndef PRIiFAST64
static_assert(sizeof(PRIiFAST64) > 0, "<inttypes.h> PRIiFAST64");
#endif
#ifndef PRIiMAX
static_assert(sizeof(PRIiMAX) > 0, "<inttypes.h> PRIiMAX");
#endif
#ifndef PRIiPTR
static_assert(sizeof(PRIiPTR) > 0, "<inttypes.h> PRIiPTR");
#endif
#ifndef PRIo8
static_assert(sizeof(PRIo8) > 0, "<inttypes.h> PRIo8");
#endif
#ifndef PRIo16
static_assert(sizeof(PRIo16) > 0, "<inttypes.h> PRIo16");
#endif
#ifndef PRIo32
static_assert(sizeof(PRIo32) > 0, "<inttypes.h> PRIo32");
#endif
#ifndef PRIo64
static_assert(sizeof(PRIo64) > 0, "<inttypes.h> PRIo64");
#endif
#ifndef PRIoLEAST8
static_assert(sizeof(PRIoLEAST8) > 0, "<inttypes.h> PRIoLEAST8");
#endif
#ifndef PRIoLEAST16
static_assert(sizeof(PRIoLEAST16) > 0, "<inttypes.h> PRIoLEAST16");
#endif
#ifndef PRIoLEAST32
static_assert(sizeof(PRIoLEAST32) > 0, "<inttypes.h> PRIoLEAST32");
#endif
#ifndef PRIoLEAST64
static_assert(sizeof(PRIoLEAST64) > 0, "<inttypes.h> PRIoLEAST64");
#endif
#ifndef PRIoFAST8
static_assert(sizeof(PRIoFAST8) > 0, "<inttypes.h> PRIoFAST8");
#endif
#ifndef PRIoFAST16
static_assert(sizeof(PRIoFAST16) > 0, "<inttypes.h> PRIoFAST16");
#endif
#ifndef PRIoFAST32
static_assert(sizeof(PRIoFAST32) > 0, "<inttypes.h> PRIoFAST32");
#endif
#ifndef PRIoFAST64
static_assert(sizeof(PRIoFAST64) > 0, "<inttypes.h> PRIoFAST64");
#endif
#ifndef PRIoMAX
static_assert(sizeof(PRIoMAX) > 0, "<inttypes.h> PRIoMAX");
#endif
#ifndef PRIoPTR
static_assert(sizeof(PRIoPTR) > 0, "<inttypes.h> PRIoPTR");
#endif
#ifndef PRIu8
static_assert(sizeof(PRIu8) > 0, "<inttypes.h> PRIu8");
#endif
#ifndef PRIu16
static_assert(sizeof(PRIu16) > 0, "<inttypes.h> PRIu16");
#endif
#ifndef PRIu32
static_assert(sizeof(PRIu32) > 0, "<inttypes.h> PRIu32");
#endif
#ifndef PRIu64
static_assert(sizeof(PRIu64) > 0, "<inttypes.h> PRIu64");
#endif
#ifndef PRIuLEAST8
static_assert(sizeof(PRIuLEAST8) > 0, "<inttypes.h> PRIuLEAST8");
#endif
#ifndef PRIuLEAST16
static_assert(sizeof(PRIuLEAST16) > 0, "<inttypes.h> PRIuLEAST16");
#endif
#ifndef PRIuLEAST32
static_assert(sizeof(PRIuLEAST32) > 0, "<inttypes.h> PRIuLEAST32");
#endif
#ifndef PRIuLEAST64
static_assert(sizeof(PRIuLEAST64) > 0, "<inttypes.h> PRIuLEAST64");
#endif
#ifndef PRIuFAST8
static_assert(sizeof(PRIuFAST8) > 0, "<inttypes.h> PRIuFAST8");
#endif
#ifndef PRIuFAST16
static_assert(sizeof(PRIuFAST16) > 0, "<inttypes.h> PRIuFAST16");
#endif
#ifndef PRIuFAST32
static_assert(sizeof(PRIuFAST32) > 0, "<inttypes.h> PRIuFAST32");
#endif
#ifndef PRIuFAST64
static_assert(sizeof(PRIuFAST64) > 0, "<inttypes.h> PRIuFAST64");
#endif
#ifndef PRIuMAX
static_assert(sizeof(PRIuMAX) > 0, "<inttypes.h> PRIuMAX");
#endif
#ifndef PRIuPTR
static_assert(sizeof(PRIuPTR) > 0, "<inttypes.h> PRIuPTR");
#endif
#ifndef PRIx8
static_assert(sizeof(PRIx8) > 0, "<inttypes.h> PRIx8");
#endif
#ifndef PRIx16
static_assert(sizeof(PRIx16) > 0, "<inttypes.h> PRIx16");
#endif
#ifndef PRIx32
static_assert(sizeof(PRIx32) > 0, "<inttypes.h> PRIx32");
#endif
#ifndef PRIx64
static_assert(sizeof(PRIx64) > 0, "<inttypes.h> PRIx64");
#endif
#ifndef PRIxLEAST8
static_assert(sizeof(PRIxLEAST8) > 0, "<inttypes.h> PRIxLEAST8");
#endif
#ifndef PRIxLEAST16
static_assert(sizeof(PRIxLEAST16) > 0, "<inttypes.h> PRIxLEAST16");
#endif
#ifndef PRIxLEAST32
static_assert(sizeof(PRIxLEAST32) > 0, "<inttypes.h> PRIxLEAST32");
#endif
#ifndef PRIxLEAST64
static_assert(sizeof(PRIxLEAST64) > 0, "<inttypes.h> PRIxLEAST64");
#endif
#ifndef PRIxFAST8
static_assert(sizeof(PRIxFAST8) > 0, "<inttypes.h> PRIxFAST8");
#endif
#ifndef PRIxFAST16
static_assert(sizeof(PRIxFAST16) > 0, "<inttypes.h> PRIxFAST16");
#endif
#ifndef PRIxFAST32
static_assert(sizeof(PRIxFAST32) > 0, "<inttypes.h> PRIxFAST32");
#endif
#ifndef PRIxFAST64
static_assert(sizeof(PRIxFAST64) > 0, "<inttypes.h> PRIxFAST64");
#endif
#ifndef PRIxMAX
static_assert(sizeof(PRIxMAX) > 0, "<inttypes.h> PRIxMAX");
#endif
#ifndef PRIxPTR
static_assert(sizeof(PRIxPTR) > 0, "<inttypes.h> PRIxPTR");
#endif
#ifndef PRIX8
static_assert(sizeof(PRIX8) > 0, "<inttypes.h> PRIX8");
#endif
#ifndef PRIX16
static_assert(sizeof(PRIX16) > 0, "<inttypes.h> PRIX16");
#endif
#ifndef PRIX32
static_assert(sizeof(PRIX32) > 0, "<inttypes.h> PRIX32");
#endif
#ifndef PRIX64
static_assert(sizeof(PRIX64) > 0, "<inttypes.h> PRIX64");
#endif
#ifndef PRIXLEAST8
static_assert(sizeof(PRIXLEAST8) > 0, "<inttypes.h> PRIXLEAST8");
#endif
#ifndef PRIXLEAST16
static_assert(sizeof(PRIXLEAST16) > 0, "<inttypes.h> PRIXLEAST16");
#endif
#ifndef PRIXLEAST32
static_assert(sizeof(PRIXLEAST32) > 0, "<inttypes.h> PRIXLEAST32");
#endif
#ifndef PRIXLEAST64
static_assert(sizeof(PRIXLEAST64) > 0, "<inttypes.h> PRIXLEAST64");
#endif
#ifndef PRIXFAST8
static_assert(sizeof(PRIXFAST8) > 0, "<inttypes.h> PRIXFAST8");
#endif
#ifndef PRIXFAST16
static_assert(sizeof(PRIXFAST16) > 0, "<inttypes.h> PRIXFAST16");
#endif
#ifndef PRIXFAST32
static_assert(sizeof(PRIXFAST32) > 0, "<inttypes.h> PRIXFAST32");
#endif
#ifndef PRIXFAST64
static_assert(sizeof(PRIXFAST64) > 0, "<inttypes.h> PRIXFAST64");
#endif
#ifndef PRIXMAX
static_assert(sizeof(PRIXMAX) > 0, "<inttypes.h> PRIXMAX");
#endif
#ifndef PRIXPTR
static_assert(sizeof(PRIXPTR) > 0, "<inttypes.h> PRIXPTR");
#endif
#ifndef SCNb8
static_assert(sizeof(SCNb8) > 0, "<inttypes.h> SCNb8");
#endif
#ifndef SCNb16
static_assert(sizeof(SCNb16) > 0, "<inttypes.h> SCNb16");
#endif
#ifndef SCNb32
static_assert(sizeof(SCNb32) > 0, "<inttypes.h> SCNb32");
#endif
#ifndef SCNb64
static_assert(sizeof(SCNb64) > 0, "<inttypes.h> SCNb64");
#endif
#ifndef SCNbLEAST8
static_assert(sizeof(SCNbLEAST8) > 0, "<inttypes.h> SCNbLEAST8");
#endif
#ifndef SCNbLEAST16
static_assert(sizeof(SCNbLEAST16) > 0, "<inttypes.h> SCNbLEAST16");
#endif
#ifndef SCNbLEAST32
static_assert(sizeof(SCNbLEAST32) > 0, "<inttypes.h> SCNbLEAST32");
#endif
#ifndef SCNbLEAST64
static_assert(sizeof(SCNbLEAST64) > 0, "<inttypes.h> SCNbLEAST64");
#endif
#ifndef SCNbFAST8
static_assert(sizeof(SCNbFAST8) > 0, "<inttypes.h> SCNbFAST8");
#endif
#ifndef SCNbFAST16
static_assert(sizeof(SCNbFAST16) > 0, "<inttypes.h> SCNbFAST16");
#endif
#ifndef SCNbFAST32
static_assert(sizeof(SCNbFAST32) > 0, "<inttypes.h> SCNbFAST32");
#endif
#ifndef SCNbFAST64
static_assert(sizeof(SCNbFAST64) > 0, "<inttypes.h> SCNbFAST64");
#endif
#ifndef SCNbMAX
static_assert(sizeof(SCNbMAX) > 0, "<inttypes.h> SCNbMAX");
#endif
#ifndef SCNbPTR
static_assert(sizeof(SCNbPTR) > 0, "<inttypes.h> SCNbPTR");
#endif
#ifndef SCNd8
static_assert(sizeof(SCNd8) > 0, "<inttypes.h> SCNd8");
#endif
#ifndef SCNd16
static_assert(sizeof(SCNd16) > 0, "<inttypes.h> SCNd16");
#endif
#ifndef SCNd32
static_assert(sizeof(SCNd32) > 0, "<inttypes.h> SCNd32");
#endif
#ifndef SCNd64
static_assert(sizeof(SCNd64) > 0, "<inttypes.h> SCNd64");
#endif
#ifndef SCNdLEAST8
static_assert(sizeof(SCNdLEAST8) > 0, "<inttypes.h> SCNdLEAST8");
#endif
#ifndef SCNdLEAST16
static_assert(sizeof(SCNdLEAST16) > 0, "<inttypes.h> SCNdLEAST16");
#endif
#ifndef SCNdLEAST32
static_assert(sizeof(SCNdLEAST32) > 0, "<inttypes.h> SCNdLEAST32");
#endif
#ifndef SCNdLEAST64
static_assert(sizeof(SCNdLEAST64) > 0, "<inttypes.h> SCNdLEAST64");
#endif
#ifndef SCNdFAST8
static_assert(sizeof(SCNdFAST8) > 0, "<inttypes.h> SCNdFAST8");
#endif
#ifndef SCNdFAST16
static_assert(sizeof(SCNdFAST16) > 0, "<inttypes.h> SCNdFAST16");
#endif
#ifndef SCNdFAST32
static_assert(sizeof(SCNdFAST32) > 0, "<inttypes.h> SCNdFAST32");
#endif
#ifndef SCNdFAST64
static_assert(sizeof(SCNdFAST64) > 0, "<inttypes.h> SCNdFAST64");
#endif
#ifndef SCNdMAX
static_assert(sizeof(SCNdMAX) > 0, "<inttypes.h> SCNdMAX");
#endif
#ifndef SCNdPTR
static_assert(sizeof(SCNdPTR) > 0, "<inttypes.h> SCNdPTR");
#endif
#ifndef SCNi8
static_assert(sizeof(SCNi8) > 0, "<inttypes.h> SCNi8");
#endif
#ifndef SCNi16
static_assert(sizeof(SCNi16) > 0, "<inttypes.h> SCNi16");
#endif
#ifndef SCNi32
static_assert(sizeof(SCNi32) > 0, "<inttypes.h> SCNi32");
#endif
#ifndef SCNi64
static_assert(sizeof(SCNi64) > 0, "<inttypes.h> SCNi64");
#endif
#ifndef SCNiLEAST8
static_assert(sizeof(SCNiLEAST8) > 0, "<inttypes.h> SCNiLEAST8");
#endif
#ifndef SCNiLEAST16
static_assert(sizeof(SCNiLEAST16) > 0, "<inttypes.h> SCNiLEAST16");
#endif
#ifndef SCNiLEAST32
static_assert(sizeof(SCNiLEAST32) > 0, "<inttypes.h> SCNiLEAST32");
#endif
#ifndef SCNiLEAST64
static_assert(sizeof(SCNiLEAST64) > 0, "<inttypes.h> SCNiLEAST64");
#endif
#ifndef SCNiFAST8
static_assert(sizeof(SCNiFAST8) > 0, "<inttypes.h> SCNiFAST8");
#endif
#ifndef SCNiFAST16
static_assert(sizeof(SCNiFAST16) > 0, "<inttypes.h> SCNiFAST16");
#endif
#ifndef SCNiFAST32
static_assert(sizeof(SCNiFAST32) > 0, "<inttypes.h> SCNiFAST32");
#endif
#ifndef SCNiFAST64
static_assert(sizeof(SCNiFAST64) > 0, "<inttypes.h> SCNiFAST64");
#endif
#ifndef SCNiMAX
static_assert(sizeof(SCNiMAX) > 0, "<inttypes.h> SCNiMAX");
#endif
#ifndef SCNiPTR
static_assert(sizeof(SCNiPTR) > 0, "<inttypes.h> SCNiPTR");
#endif
#ifndef SCNo8
static_assert(sizeof(SCNo8) > 0, "<inttypes.h> SCNo8");
#endif
#ifndef SCNo16
static_assert(sizeof(SCNo16) > 0, "<inttypes.h> SCNo16");
#endif
#ifndef SCNo32
static_assert(sizeof(SCNo32) > 0, "<inttypes.h> SCNo32");
#endif
#ifndef SCNo64
static_assert(sizeof(SCNo64) > 0, "<inttypes.h> SCNo64");
#endif
#ifndef SCNoLEAST8
static_assert(sizeof(SCNoLEAST8) > 0, "<inttypes.h> SCNoLEAST8");
#endif
#ifndef SCNoLEAST16
static_assert(sizeof(SCNoLEAST16) > 0, "<inttypes.h> SCNoLEAST16");
#endif
#ifndef SCNoLEAST32
static_assert(sizeof(SCNoLEAST32) > 0, "<inttypes.h> SCNoLEAST32");
#endif
#ifndef SCNoLEAST64
static_assert(sizeof(SCNoLEAST64) > 0, "<inttypes.h> SCNoLEAST64");
#endif
#ifndef SCNoFAST8
static_assert(sizeof(SCNoFAST8) > 0, "<inttypes.h> SCNoFAST8");
#endif
#ifndef SCNoFAST16
static_assert(sizeof(SCNoFAST16) > 0, "<inttypes.h> SCNoFAST16");
#endif
#ifndef SCNoFAST32
static_assert(sizeof(SCNoFAST32) > 0, "<inttypes.h> SCNoFAST32");
#endif
#ifndef SCNoFAST64
static_assert(sizeof(SCNoFAST64) > 0, "<inttypes.h> SCNoFAST64");
#endif
#ifndef SCNoMAX
static_assert(sizeof(SCNoMAX) > 0, "<inttypes.h> SCNoMAX");
#endif
#ifndef SCNoPTR
static_assert(sizeof(SCNoPTR) > 0, "<inttypes.h> SCNoPTR");
#endif
#ifndef SCNu8
static_assert(sizeof(SCNu8) > 0, "<inttypes.h> SCNu8");
#endif
#ifndef SCNu16
static_assert(sizeof(SCNu16) > 0, "<inttypes.h> SCNu16");
#endif
#ifndef SCNu32
static_assert(sizeof(SCNu32) > 0, "<inttypes.h> SCNu32");
#endif
#ifndef SCNu64
static_assert(sizeof(SCNu64) > 0, "<inttypes.h> SCNu64");
#endif
#ifndef SCNuLEAST8
static_assert(sizeof(SCNuLEAST8) > 0, "<inttypes.h> SCNuLEAST8");
#endif
#ifndef SCNuLEAST16
static_assert(sizeof(SCNuLEAST16) > 0, "<inttypes.h> SCNuLEAST16");
#endif
#ifndef SCNuLEAST32
static_assert(sizeof(SCNuLEAST32) > 0, "<inttypes.h> SCNuLEAST32");
#endif
#ifndef SCNuLEAST64
static_assert(sizeof(SCNuLEAST64) > 0, "<inttypes.h> SCNuLEAST64");
#endif
#ifndef SCNuFAST8
static_assert(sizeof(SCNuFAST8) > 0, "<inttypes.h> SCNuFAST8");
#endif
#ifndef SCNuFAST16
static_assert(sizeof(SCNuFAST16) > 0, "<inttypes.h> SCNuFAST16");
#endif
#ifndef SCNuFAST32
static_assert(sizeof(SCNuFAST32) > 0, "<inttypes.h> SCNuFAST32");
#endif
#ifndef SCNuFAST64
static_assert(sizeof(SCNuFAST64) > 0, "<inttypes.h> SCNuFAST64");
#endif
#ifndef SCNuMAX
static_assert(sizeof(SCNuMAX) > 0, "<inttypes.h> SCNuMAX");
#endif
#ifndef SCNuPTR
static_assert(sizeof(SCNuPTR) > 0, "<inttypes.h> SCNuPTR");
#endif
#ifndef SCNx8
static_assert(sizeof(SCNx8) > 0, "<inttypes.h> SCNx8");
#endif
#ifndef SCNx16
static_assert(sizeof(SCNx16) > 0, "<inttypes.h> SCNx16");
#endif
#ifndef SCNx32
static_assert(sizeof(SCNx32) > 0, "<inttypes.h> SCNx32");
#endif
#ifndef SCNx64
static_assert(sizeof(SCNx64) > 0, "<inttypes.h> SCNx64");
#endif
#ifndef SCNxLEAST8
static_assert(sizeof(SCNxLEAST8) > 0, "<inttypes.h> SCNxLEAST8");
#endif
#ifndef SCNxLEAST16
static_assert(sizeof(SCNxLEAST16) > 0, "<inttypes.h> SCNxLEAST16");
#endif
#ifndef SCNxLEAST32
static_assert(sizeof(SCNxLEAST32) > 0, "<inttypes.h> SCNxLEAST32");
#endif
#ifndef SCNxLEAST64
static_assert(sizeof(SCNxLEAST64) > 0, "<inttypes.h> SCNxLEAST64");
#endif
#ifndef SCNxFAST8
static_assert(sizeof(SCNxFAST8) > 0, "<inttypes.h> SCNxFAST8");
#endif
#ifndef SCNxFAST16
static_assert(sizeof(SCNxFAST16) > 0, "<inttypes.h> SCNxFAST16");
#endif
#ifndef SCNxFAST32
static_assert(sizeof(SCNxFAST32) > 0, "<inttypes.h> SCNxFAST32");
#endif
#ifndef SCNxFAST64
static_assert(sizeof(SCNxFAST64) > 0, "<inttypes.h> SCNxFAST64");
#endif
#ifndef SCNxMAX
static_assert(sizeof(SCNxMAX) > 0, "<inttypes.h> SCNxMAX");
#endif
#ifndef SCNxPTR
static_assert(sizeof(SCNxPTR) > 0, "<inttypes.h> SCNxPTR");
#endif
#ifndef and
static_assert(sizeof(and) > 0, "<iso646.h> and");
#endif
#ifndef bitor
static_assert(sizeof(bitor) > 0, "<iso646.h> bitor");
#endif
#ifndef not_eq
static_assert(sizeof(not_eq) > 0, "<iso646.h> not_eq");
#endif
#ifndef xor
static_assert(sizeof(xor) > 0, "<iso646.h> xor");
#endif
#ifndef and_eq
static_assert(sizeof(and_eq) > 0, "<iso646.h> and_eq");
#endif
#ifndef compl
static_assert(sizeof(compl ) > 0, "<iso646.h> compl");
#endif
#ifndef or
static_assert(sizeof(or) > 0, "<iso646.h> or");
#endif
#ifndef xor_eq
static_assert(sizeof(xor_eq) > 0, "<iso646.h> xor_eq");
#endif
#ifndef bitand
static_assert(sizeof(bitand) > 0, "<iso646.h> bitand");
#endif
#ifndef not
static_assert(sizeof(not ) > 0, "<iso646.h> not");
#endif
#ifndef or_eq
static_assert(sizeof(or_eq) > 0, "<iso646.h> or_eq");
#endif
#ifndef __STDC_VERSION_LIMITS_H__
static_assert(sizeof(__STDC_VERSION_LIMITS_H__) > 0, "<limits.h> __STDC_VERSION_LIMITS_H__");
#endif
#ifndef ULONG_WIDTH
static_assert(sizeof(ULONG_WIDTH) > 0, "<limits.h> ULONG_WIDTH");
#endif
#ifndef USHRT_MAX
static_assert(sizeof(USHRT_MAX) > 0, "<limits.h> USHRT_MAX");
#endif
#if !defined(BITINT_MAXWIDTH) && defined(__BITINT_MAXWIDTH__)
static_assert(sizeof(BITINT_MAXWIDTH) > 0, "<limits.h> BITINT_MAXWIDTH");
#endif
#ifndef LLONG_WIDTH
static_assert(sizeof(LLONG_WIDTH) > 0, "<limits.h> LLONG_WIDTH");
#endif
#ifndef INT_MIN
static_assert(sizeof(INT_MIN) > 0, "<limits.h> INT_MIN");
#endif
#ifndef BOOL_WIDTH
static_assert(sizeof(BOOL_WIDTH) > 0, "<limits.h> BOOL_WIDTH");
#endif
#ifndef ULLONG_WIDTH
static_assert(sizeof(ULLONG_WIDTH) > 0, "<limits.h> ULLONG_WIDTH");
#endif
#ifndef INT_MAX
static_assert(sizeof(INT_MAX) > 0, "<limits.h> INT_MAX");
#endif
#ifndef CHAR_BIT
static_assert(sizeof(CHAR_BIT) > 0, "<limits.h> CHAR_BIT");
#endif
#ifndef BOOL_MAX
static_assert(sizeof(BOOL_MAX) > 0, "<limits.h> BOOL_MAX");
#endif
#ifndef UINT_MAX
static_assert(sizeof(UINT_MAX) > 0, "<limits.h> UINT_MAX");
#endif
#ifndef CHAR_WIDTH
static_assert(sizeof(CHAR_WIDTH) > 0, "<limits.h> CHAR_WIDTH");
#endif
#ifndef SCHAR_MIN
static_assert(sizeof(SCHAR_MIN) > 0, "<limits.h> SCHAR_MIN");
#endif
#ifndef LONG_MIN
static_assert(sizeof(LONG_MIN) > 0, "<limits.h> LONG_MIN");
#endif
#ifndef SCHAR_WIDTH
static_assert(sizeof(SCHAR_WIDTH) > 0, "<limits.h> SCHAR_WIDTH");
#endif
#ifndef SCHAR_MAX
static_assert(sizeof(SCHAR_MAX) > 0, "<limits.h> SCHAR_MAX");
#endif
#ifndef LONG_MAX
static_assert(sizeof(LONG_MAX) > 0, "<limits.h> LONG_MAX");
#endif
#ifndef UCHAR_WIDTH
static_assert(sizeof(UCHAR_WIDTH) > 0, "<limits.h> UCHAR_WIDTH");
#endif
#ifndef UCHAR_MAX
static_assert(sizeof(UCHAR_MAX) > 0, "<limits.h> UCHAR_MAX");
#endif
#ifndef ULONG_MAX
static_assert(sizeof(ULONG_MAX) > 0, "<limits.h> ULONG_MAX");
#endif
#ifndef SHRT_WIDTH
static_assert(sizeof(SHRT_WIDTH) > 0, "<limits.h> SHRT_WIDTH");
#endif
#ifndef CHAR_MIN
static_assert(sizeof(CHAR_MIN) > 0, "<limits.h> CHAR_MIN");
#endif
#ifndef LLONG_MIN
static_assert(sizeof(LLONG_MIN) > 0, "<limits.h> LLONG_MIN");
#endif
#ifndef USHRT_WIDTH
static_assert(sizeof(USHRT_WIDTH) > 0, "<limits.h> USHRT_WIDTH");
#endif
#ifndef CHAR_MAX
static_assert(sizeof(CHAR_MAX) > 0, "<limits.h> CHAR_MAX");
#endif
#ifndef LLONG_MAX
static_assert(sizeof(LLONG_MAX) > 0, "<limits.h> LLONG_MAX");
#endif
#ifndef INT_WIDTH
static_assert(sizeof(INT_WIDTH) > 0, "<limits.h> INT_WIDTH");
#endif
#ifndef MB_LEN_MAX
static_assert(sizeof(MB_LEN_MAX) > 0, "<limits.h> MB_LEN_MAX");
#endif
#ifndef ULLONG_MAX
static_assert(sizeof(ULLONG_MAX) > 0, "<limits.h> ULLONG_MAX");
#endif
#ifndef UINT_WIDTH
static_assert(sizeof(UINT_WIDTH) > 0, "<limits.h> UINT_WIDTH");
#endif
#ifndef SHRT_MIN
static_assert(sizeof(SHRT_MIN) > 0, "<limits.h> SHRT_MIN");
#endif
#ifndef LONG_WIDTH
static_assert(sizeof(LONG_WIDTH) > 0, "<limits.h> LONG_WIDTH");
#endif
#ifndef SHRT_MAX
static_assert(sizeof(SHRT_MAX) > 0, "<limits.h> SHRT_MAX");
#endif
static_assert(sizeof(struct lconv) > 0, "<locale.h> struct lconv");
#ifndef LC_ALL
static_assert(sizeof(LC_ALL) > 0, "<locale.h> LC_ALL");
#endif
#ifndef LC_CTYPE
static_assert(sizeof(LC_CTYPE) > 0, "<locale.h> LC_CTYPE");
#endif
#ifndef LC_NUMERIC
static_assert(sizeof(LC_NUMERIC) > 0, "<locale.h> LC_NUMERIC");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<locale.h> NULL");
#endif
#ifndef LC_COLLATE
static_assert(sizeof(LC_COLLATE) > 0, "<locale.h> LC_COLLATE");
#endif
#ifndef LC_MONETARY
static_assert(sizeof(LC_MONETARY) > 0, "<locale.h> LC_MONETARY");
#endif
#ifndef LC_TIME
static_assert(sizeof(LC_TIME) > 0, "<locale.h> LC_TIME");
#endif
#ifndef __STDC_VERSION_MATH_H__
static_assert(sizeof(__STDC_VERSION_MATH_H__) > 0, "<math.h> __STDC_VERSION_MATH_H__");
#endif
#ifndef FP_INT_TONEARESTFROMZERO
static_assert(sizeof(FP_INT_TONEARESTFROMZERO) > 0, "<math.h> FP_INT_TONEARESTFROMZERO");
#endif
#ifndef float_t
static_assert(sizeof(float_t) > 0, "<math.h> float_t");
#endif
#ifndef FP_INT_TONEAREST
static_assert(sizeof(FP_INT_TONEAREST) > 0, "<math.h> FP_INT_TONEAREST");
#endif
#ifndef double_t
static_assert(sizeof(double_t) > 0, "<math.h> double_t");
#endif
#ifndef HUGE_VAL
static_assert(sizeof(HUGE_VAL) > 0, "<math.h> HUGE_VAL");
#endif
#ifndef HUGE_VALF
static_assert(sizeof(HUGE_VALF) > 0, "<math.h> HUGE_VALF");
#endif
#ifndef HUGE_VALL
static_assert(sizeof(HUGE_VALL) > 0, "<math.h> HUGE_VALL");
#endif
#ifndef INFINITY
static_assert(sizeof(INFINITY) > 0, "<math.h> INFINITY");
#endif
#ifndef NAN
static_assert(sizeof(NAN) > 0, "<math.h> NAN");
#endif
#ifndef FP_ILOGB0
static_assert(sizeof(FP_ILOGB0) > 0, "<math.h> FP_ILOGB0");
#endif
#ifndef FP_INFINITE
static_assert(sizeof(FP_INFINITE) > 0, "<math.h> FP_INFINITE");
#endif
#ifndef FP_ILOGBNAN
static_assert(sizeof(FP_ILOGBNAN) > 0, "<math.h> FP_ILOGBNAN");
#endif
#ifndef FP_NAN
static_assert(sizeof(FP_NAN) > 0, "<math.h> FP_NAN");
#endif
#ifndef FP_LLOGB0
static_assert(sizeof(FP_LLOGB0) > 0, "<math.h> FP_LLOGB0");
#endif
#ifndef FP_NORMAL
static_assert(sizeof(FP_NORMAL) > 0, "<math.h> FP_NORMAL");
#endif
#ifndef FP_LLOGBNAN
static_assert(sizeof(FP_LLOGBNAN) > 0, "<math.h> FP_LLOGBNAN");
#endif
#ifndef FP_SUBNORMAL
static_assert(sizeof(FP_SUBNORMAL) > 0, "<math.h> FP_SUBNORMAL");
#endif
#ifndef MATH_ERRNO
static_assert(sizeof(MATH_ERRNO) > 0, "<math.h> MATH_ERRNO");
#endif
#ifndef FP_ZERO
static_assert(sizeof(FP_ZERO) > 0, "<math.h> FP_ZERO");
#endif
#ifndef MATH_ERREXCEPT
static_assert(sizeof(MATH_ERREXCEPT) > 0, "<math.h> MATH_ERREXCEPT");
#endif
#ifndef FP_INT_UPWARD
static_assert(sizeof(FP_INT_UPWARD) > 0, "<math.h> FP_INT_UPWARD");
#endif
#ifndef math_errhandling
static_assert(sizeof(math_errhandling) > 0, "<math.h> math_errhandling");
#endif
#ifndef FP_INT_DOWNWARD
static_assert(sizeof(FP_INT_DOWNWARD) > 0, "<math.h> FP_INT_DOWNWARD");
#endif
#ifndef FP_INT_TOWARDZERO
static_assert(sizeof(FP_INT_TOWARDZERO) > 0, "<math.h> FP_INT_TOWARDZERO");
#endif
#ifndef __STDC_VERSION_SETJMP_H__
static_assert(sizeof(__STDC_VERSION_SETJMP_H__) > 0, "<setjmp.h> __STDC_VERSION_SETJMP_H__");
#endif
#ifndef jmp_buf
static_assert(sizeof(jmp_buf) > 0, "<setjmp.h> jmp_buf");
#endif
#ifndef sig_atomic_t
static_assert(sizeof(sig_atomic_t) > 0, "<signal.h> sig_atomic_t");
#endif
#ifndef SIG_IGN
static_assert(sizeof(SIG_IGN) > 0, "<signal.h> SIG_IGN");
#endif
#ifndef SIGILL
static_assert(sizeof(SIGILL) > 0, "<signal.h> SIGILL");
#endif
#ifndef SIGTERM
static_assert(sizeof(SIGTERM) > 0, "<signal.h> SIGTERM");
#endif
#ifndef SIG_DFL
static_assert(sizeof(SIG_DFL) > 0, "<signal.h> SIG_DFL");
#endif
#ifndef SIGABRT
static_assert(sizeof(SIGABRT) > 0, "<signal.h> SIGABRT");
#endif
#ifndef SIGINT
static_assert(sizeof(SIGINT) > 0, "<signal.h> SIGINT");
#endif
#ifndef SIG_ERR
static_assert(sizeof(SIG_ERR) > 0, "<signal.h> SIG_ERR");
#endif
#ifndef SIGFPE
static_assert(sizeof(SIGFPE) > 0, "<signal.h> SIGFPE");
#endif
#ifndef SIGSEGV
static_assert(sizeof(SIGSEGV) > 0, "<signal.h> SIGSEGV");
#endif
#ifndef va_list
static_assert(sizeof(va_list) > 0, "<stdarg.h> va_list");
#endif
#ifndef __STDC_VERSION_STDARG_H__
static_assert(sizeof(__STDC_VERSION_STDARG_H__) > 0, "<stdarg.h> __STDC_VERSION_STDARG_H__");
#endif
#ifndef atomic_flag
static_assert(sizeof(atomic_flag) > 0, "<stdatomic.h> atomic_flag");
#endif
#ifndef __STDC_VERSION_STDATOMIC_H__
static_assert(sizeof(__STDC_VERSION_STDATOMIC_H__) > 0, "<stdatomic.h> __STDC_VERSION_STDATOMIC_H__");
#endif
#ifndef memory_order_relaxed
static_assert(sizeof(memory_order_relaxed) > 0, "<stdatomic.h> memory_order_relaxed");
#endif
#ifndef ATOMIC_BOOL_LOCK_FREE
static_assert(sizeof(ATOMIC_BOOL_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_BOOL_LOCK_FREE");
#endif
#ifndef memory_order_consume
static_assert(sizeof(memory_order_consume) > 0, "<stdatomic.h> memory_order_consume");
#endif
#ifndef ATOMIC_CHAR_LOCK_FREE
static_assert(sizeof(ATOMIC_CHAR_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_CHAR_LOCK_FREE");
#endif
#ifndef memory_order_acquire
static_assert(sizeof(memory_order_acquire) > 0, "<stdatomic.h> memory_order_acquire");
#endif
#ifndef ATOMIC_CHAR8_T_LOCK_FREE
static_assert(sizeof(ATOMIC_CHAR8_T_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_CHAR8_T_LOCK_FREE");
#endif
#ifndef memory_order_release
static_assert(sizeof(memory_order_release) > 0, "<stdatomic.h> memory_order_release");
#endif
#ifndef ATOMIC_CHAR16_T_LOCK_FREE
static_assert(sizeof(ATOMIC_CHAR16_T_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_CHAR16_T_LOCK_FREE");
#endif
#ifndef memory_order_acq_rel
static_assert(sizeof(memory_order_acq_rel) > 0, "<stdatomic.h> memory_order_acq_rel");
#endif
#ifndef ATOMIC_CHAR32_T_LOCK_FREE
static_assert(sizeof(ATOMIC_CHAR32_T_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_CHAR32_T_LOCK_FREE");
#endif
#ifndef memory_order_seq_cst
static_assert(sizeof(memory_order_seq_cst) > 0, "<stdatomic.h> memory_order_seq_cst");
#endif
#ifndef ATOMIC_WCHAR_T_LOCK_FREE
static_assert(sizeof(ATOMIC_WCHAR_T_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_WCHAR_T_LOCK_FREE");
#endif
#ifndef atomic_bool
static_assert(sizeof(atomic_bool) > 0, "<stdatomic.h> atomic_bool");
#endif
#ifndef ATOMIC_SHORT_LOCK_FREE
static_assert(sizeof(ATOMIC_SHORT_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_SHORT_LOCK_FREE");
#endif
#ifndef atomic_char
static_assert(sizeof(atomic_char) > 0, "<stdatomic.h> atomic_char");
#endif
#ifndef ATOMIC_INT_LOCK_FREE
static_assert(sizeof(ATOMIC_INT_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_INT_LOCK_FREE");
#endif
#ifndef atomic_schar
static_assert(sizeof(atomic_schar) > 0, "<stdatomic.h> atomic_schar");
#endif
#ifndef ATOMIC_LONG_LOCK_FREE
static_assert(sizeof(ATOMIC_LONG_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_LONG_LOCK_FREE");
#endif
#ifndef atomic_uchar
static_assert(sizeof(atomic_uchar) > 0, "<stdatomic.h> atomic_uchar");
#endif
#ifndef ATOMIC_LLONG_LOCK_FREE
static_assert(sizeof(ATOMIC_LLONG_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_LLONG_LOCK_FREE");
#endif
#ifndef atomic_short
static_assert(sizeof(atomic_short) > 0, "<stdatomic.h> atomic_short");
#endif
#ifndef ATOMIC_POINTER_LOCK_FREE
static_assert(sizeof(ATOMIC_POINTER_LOCK_FREE) > 0, "<stdatomic.h> ATOMIC_POINTER_LOCK_FREE");
#endif
#ifndef atomic_ushort
static_assert(sizeof(atomic_ushort) > 0, "<stdatomic.h> atomic_ushort");
#endif
#ifndef ATOMIC_FLAG_INIT
static_assert(sizeof(ATOMIC_FLAG_INIT) > 0, "<stdatomic.h> ATOMIC_FLAG_INIT");
#endif
#ifndef atomic_int
static_assert(sizeof(atomic_int) > 0, "<stdatomic.h> atomic_int");
#endif
#ifndef memory_order
static_assert(sizeof(memory_order) > 0, "<stdatomic.h> memory_order");
#endif
#ifndef atomic_uint
static_assert(sizeof(atomic_uint) > 0, "<stdatomic.h> atomic_uint");
#endif
#ifndef atomic_long
static_assert(sizeof(atomic_long) > 0, "<stdatomic.h> atomic_long");
#endif
#ifndef atomic_uint_least64_t
static_assert(sizeof(atomic_uint_least64_t) > 0, "<stdatomic.h> atomic_uint_least64_t");
#endif
#ifndef atomic_ulong
static_assert(sizeof(atomic_ulong) > 0, "<stdatomic.h> atomic_ulong");
#endif
#ifndef atomic_int_fast8_t
static_assert(sizeof(atomic_int_fast8_t) > 0, "<stdatomic.h> atomic_int_fast8_t");
#endif
#ifndef atomic_llong
static_assert(sizeof(atomic_llong) > 0, "<stdatomic.h> atomic_llong");
#endif
#ifndef atomic_uint_fast8_t
static_assert(sizeof(atomic_uint_fast8_t) > 0, "<stdatomic.h> atomic_uint_fast8_t");
#endif
#ifndef atomic_ullong
static_assert(sizeof(atomic_ullong) > 0, "<stdatomic.h> atomic_ullong");
#endif
#ifndef atomic_int_fast16_t
static_assert(sizeof(atomic_int_fast16_t) > 0, "<stdatomic.h> atomic_int_fast16_t");
#endif
#ifndef atomic_char8_t
static_assert(sizeof(atomic_char8_t) > 0, "<stdatomic.h> atomic_char8_t");
#endif
#ifndef atomic_uint_fast16_t
static_assert(sizeof(atomic_uint_fast16_t) > 0, "<stdatomic.h> atomic_uint_fast16_t");
#endif
#ifndef atomic_char16_t
static_assert(sizeof(atomic_char16_t) > 0, "<stdatomic.h> atomic_char16_t");
#endif
#ifndef atomic_int_fast32_t
static_assert(sizeof(atomic_int_fast32_t) > 0, "<stdatomic.h> atomic_int_fast32_t");
#endif
#ifndef atomic_char32_t
static_assert(sizeof(atomic_char32_t) > 0, "<stdatomic.h> atomic_char32_t");
#endif
#ifndef atomic_uint_fast32_t
static_assert(sizeof(atomic_uint_fast32_t) > 0, "<stdatomic.h> atomic_uint_fast32_t");
#endif
#ifndef atomic_wchar_t
static_assert(sizeof(atomic_wchar_t) > 0, "<stdatomic.h> atomic_wchar_t");
#endif
#ifndef atomic_int_fast64_t
static_assert(sizeof(atomic_int_fast64_t) > 0, "<stdatomic.h> atomic_int_fast64_t");
#endif
#ifndef atomic_int_least8_t
static_assert(sizeof(atomic_int_least8_t) > 0, "<stdatomic.h> atomic_int_least8_t");
#endif
#ifndef atomic_uint_fast64_t
static_assert(sizeof(atomic_uint_fast64_t) > 0, "<stdatomic.h> atomic_uint_fast64_t");
#endif
#ifndef atomic_uint_least8_t
static_assert(sizeof(atomic_uint_least8_t) > 0, "<stdatomic.h> atomic_uint_least8_t");
#endif
#ifndef atomic_intptr_t
static_assert(sizeof(atomic_intptr_t) > 0, "<stdatomic.h> atomic_intptr_t");
#endif
#ifndef atomic_int_least16_t
static_assert(sizeof(atomic_int_least16_t) > 0, "<stdatomic.h> atomic_int_least16_t");
#endif
#ifndef atomic_uintptr_t
static_assert(sizeof(atomic_uintptr_t) > 0, "<stdatomic.h> atomic_uintptr_t");
#endif
#ifndef atomic_uint_least16_t
static_assert(sizeof(atomic_uint_least16_t) > 0, "<stdatomic.h> atomic_uint_least16_t");
#endif
#ifndef atomic_size_t
static_assert(sizeof(atomic_size_t) > 0, "<stdatomic.h> atomic_size_t");
#endif
#ifndef atomic_int_least32_t
static_assert(sizeof(atomic_int_least32_t) > 0, "<stdatomic.h> atomic_int_least32_t");
#endif
#ifndef atomic_ptrdiff_t
static_assert(sizeof(atomic_ptrdiff_t) > 0, "<stdatomic.h> atomic_ptrdiff_t");
#endif
#ifndef atomic_uint_least32_t
static_assert(sizeof(atomic_uint_least32_t) > 0, "<stdatomic.h> atomic_uint_least32_t");
#endif
#ifndef atomic_intmax_t
static_assert(sizeof(atomic_intmax_t) > 0, "<stdatomic.h> atomic_intmax_t");
#endif
#ifndef atomic_int_least64_t
static_assert(sizeof(atomic_int_least64_t) > 0, "<stdatomic.h> atomic_int_least64_t");
#endif
#ifndef atomic_uintmax_t
static_assert(sizeof(atomic_uintmax_t) > 0, "<stdatomic.h> atomic_uintmax_t");
#endif
#ifndef __STDC_ENDIAN_BIG__
static_assert(sizeof(__STDC_ENDIAN_BIG__) > 0, "<stdbit.h> __STDC_ENDIAN_BIG__");
#endif
#ifndef __STDC_ENDIAN_NATIVE__
static_assert(sizeof(__STDC_ENDIAN_NATIVE__) > 0, "<stdbit.h> __STDC_ENDIAN_NATIVE__");
#endif
#ifndef __STDC_ENDIAN_LITTLE__
static_assert(sizeof(__STDC_ENDIAN_LITTLE__) > 0, "<stdbit.h> __STDC_ENDIAN_LITTLE__");
#endif
#ifndef __STDC_VERSION_STDBIT_H__
static_assert(sizeof(__STDC_VERSION_STDBIT_H__) > 0, "<stdbit.h> __STDC_VERSION_STDBIT_H__");
#endif
#ifndef __bool_true_false_are_defined
static_assert(sizeof(__bool_true_false_are_defined) > 0, "<stdbool.h> __bool_true_false_are_defined");
#endif
#ifndef __STDC_VERSION_STDCKDINT_H__
static_assert(sizeof(__STDC_VERSION_STDCKDINT_H__) > 0, "<stdckdint.h> __STDC_VERSION_STDCKDINT_H__");
#endif
#ifndef ptrdiff_t
static_assert(sizeof(ptrdiff_t) > 0, "<stddef.h> ptrdiff_t");
#endif
#ifndef max_align_t
static_assert(sizeof(max_align_t) > 0, "<stddef.h> max_align_t");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<stddef.h> NULL");
#endif
#ifndef nullptr_t
static_assert(sizeof(nullptr_t) > 0, "<stddef.h> nullptr_t");
#endif
#ifndef wchar_t
static_assert(sizeof(wchar_t) > 0, "<stddef.h> wchar_t");
#endif
#ifndef size_t
static_assert(sizeof(size_t) > 0, "<stddef.h> size_t");
#endif
#ifndef __STDC_VERSION_STDDEF_H__
static_assert(sizeof(__STDC_VERSION_STDDEF_H__) > 0, "<stddef.h> __STDC_VERSION_STDDEF_H__");
#endif
#ifndef int8_t
static_assert(sizeof(int8_t) > 0, "<stdint.h> int8_t");
#endif
#ifndef int16_t
static_assert(sizeof(int16_t) > 0, "<stdint.h> int16_t");
#endif
#ifndef int32_t
static_assert(sizeof(int32_t) > 0, "<stdint.h> int32_t");
#endif
#ifndef int64_t
static_assert(sizeof(int64_t) > 0, "<stdint.h> int64_t");
#endif
#ifndef INT_LEAST8_WIDTH
static_assert(sizeof(INT_LEAST8_WIDTH) > 0, "<stdint.h> INT_LEAST8_WIDTH");
#endif
#ifndef INT_LEAST16_WIDTH
static_assert(sizeof(INT_LEAST16_WIDTH) > 0, "<stdint.h> INT_LEAST16_WIDTH");
#endif
#ifndef INT_LEAST32_WIDTH
static_assert(sizeof(INT_LEAST32_WIDTH) > 0, "<stdint.h> INT_LEAST32_WIDTH");
#endif
#ifndef INT_LEAST64_WIDTH
static_assert(sizeof(INT_LEAST64_WIDTH) > 0, "<stdint.h> INT_LEAST64_WIDTH");
#endif
#ifndef PTRDIFF_MIN
static_assert(sizeof(PTRDIFF_MIN) > 0, "<stdint.h> PTRDIFF_MIN");
#endif
#ifndef uint8_t
static_assert(sizeof(uint8_t) > 0, "<stdint.h> uint8_t");
#endif
#ifndef uint16_t
static_assert(sizeof(uint16_t) > 0, "<stdint.h> uint16_t");
#endif
#ifndef uint32_t
static_assert(sizeof(uint32_t) > 0, "<stdint.h> uint32_t");
#endif
#ifndef uint64_t
static_assert(sizeof(uint64_t) > 0, "<stdint.h> uint64_t");
#endif
#ifndef UINT_LEAST8_MAX
static_assert(sizeof(UINT_LEAST8_MAX) > 0, "<stdint.h> UINT_LEAST8_MAX");
#endif
#ifndef UINT_LEAST16_MAX
static_assert(sizeof(UINT_LEAST16_MAX) > 0, "<stdint.h> UINT_LEAST16_MAX");
#endif
#ifndef UINT_LEAST32_MAX
static_assert(sizeof(UINT_LEAST32_MAX) > 0, "<stdint.h> UINT_LEAST32_MAX");
#endif
#ifndef UINT_LEAST64_MAX
static_assert(sizeof(UINT_LEAST64_MAX) > 0, "<stdint.h> UINT_LEAST64_MAX");
#endif
#ifndef PTRDIFF_MAX
static_assert(sizeof(PTRDIFF_MAX) > 0, "<stdint.h> PTRDIFF_MAX");
#endif
#ifndef int_least8_t
static_assert(sizeof(int_least8_t) > 0, "<stdint.h> int_least8_t");
#endif
#ifndef int_least16_t
static_assert(sizeof(int_least16_t) > 0, "<stdint.h> int_least16_t");
#endif
#ifndef int_least32_t
static_assert(sizeof(int_least32_t) > 0, "<stdint.h> int_least32_t");
#endif
#ifndef int_least64_t
static_assert(sizeof(int_least64_t) > 0, "<stdint.h> int_least64_t");
#endif
#ifndef UINT_LEAST8_WIDTH
static_assert(sizeof(UINT_LEAST8_WIDTH) > 0, "<stdint.h> UINT_LEAST8_WIDTH");
#endif
#ifndef UINT_LEAST16_WIDTH
static_assert(sizeof(UINT_LEAST16_WIDTH) > 0, "<stdint.h> UINT_LEAST16_WIDTH");
#endif
#ifndef UINT_LEAST32_WIDTH
static_assert(sizeof(UINT_LEAST32_WIDTH) > 0, "<stdint.h> UINT_LEAST32_WIDTH");
#endif
#ifndef UINT_LEAST64_WIDTH
static_assert(sizeof(UINT_LEAST64_WIDTH) > 0, "<stdint.h> UINT_LEAST64_WIDTH");
#endif
#ifndef SIG_ATOMIC_MIN
static_assert(sizeof(SIG_ATOMIC_MIN) > 0, "<stdint.h> SIG_ATOMIC_MIN");
#endif
#ifndef uint_least8_t
static_assert(sizeof(uint_least8_t) > 0, "<stdint.h> uint_least8_t");
#endif
#ifndef uint_least16_t
static_assert(sizeof(uint_least16_t) > 0, "<stdint.h> uint_least16_t");
#endif
#ifndef uint_least32_t
static_assert(sizeof(uint_least32_t) > 0, "<stdint.h> uint_least32_t");
#endif
#ifndef uint_least64_t
static_assert(sizeof(uint_least64_t) > 0, "<stdint.h> uint_least64_t");
#endif
#ifndef INT_FAST8_MIN
static_assert(sizeof(INT_FAST8_MIN) > 0, "<stdint.h> INT_FAST8_MIN");
#endif
#ifndef INT_FAST16_MIN
static_assert(sizeof(INT_FAST16_MIN) > 0, "<stdint.h> INT_FAST16_MIN");
#endif
#ifndef INT_FAST32_MIN
static_assert(sizeof(INT_FAST32_MIN) > 0, "<stdint.h> INT_FAST32_MIN");
#endif
#ifndef INT_FAST64_MIN
static_assert(sizeof(INT_FAST64_MIN) > 0, "<stdint.h> INT_FAST64_MIN");
#endif
#ifndef SIG_ATOMIC_MAX
static_assert(sizeof(SIG_ATOMIC_MAX) > 0, "<stdint.h> SIG_ATOMIC_MAX");
#endif
#ifndef int_fast8_t
static_assert(sizeof(int_fast8_t) > 0, "<stdint.h> int_fast8_t");
#endif
#ifndef int_fast16_t
static_assert(sizeof(int_fast16_t) > 0, "<stdint.h> int_fast16_t");
#endif
#ifndef int_fast32_t
static_assert(sizeof(int_fast32_t) > 0, "<stdint.h> int_fast32_t");
#endif
#ifndef int_fast64_t
static_assert(sizeof(int_fast64_t) > 0, "<stdint.h> int_fast64_t");
#endif
#ifndef INT_FAST8_MAX
static_assert(sizeof(INT_FAST8_MAX) > 0, "<stdint.h> INT_FAST8_MAX");
#endif
#ifndef INT_FAST16_MAX
static_assert(sizeof(INT_FAST16_MAX) > 0, "<stdint.h> INT_FAST16_MAX");
#endif
#ifndef INT_FAST32_MAX
static_assert(sizeof(INT_FAST32_MAX) > 0, "<stdint.h> INT_FAST32_MAX");
#endif
#ifndef INT_FAST64_MAX
static_assert(sizeof(INT_FAST64_MAX) > 0, "<stdint.h> INT_FAST64_MAX");
#endif
#ifndef SIG_ATOMIC_WIDTH
static_assert(sizeof(SIG_ATOMIC_WIDTH) > 0, "<stdint.h> SIG_ATOMIC_WIDTH");
#endif
#ifndef uint_fast8_t
static_assert(sizeof(uint_fast8_t) > 0, "<stdint.h> uint_fast8_t");
#endif
#ifndef uint_fast16_t
static_assert(sizeof(uint_fast16_t) > 0, "<stdint.h> uint_fast16_t");
#endif
#ifndef uint_fast32_t
static_assert(sizeof(uint_fast32_t) > 0, "<stdint.h> uint_fast32_t");
#endif
#ifndef uint_fast64_t
static_assert(sizeof(uint_fast64_t) > 0, "<stdint.h> uint_fast64_t");
#endif
#ifndef INT_FAST8_WIDTH
static_assert(sizeof(INT_FAST8_WIDTH) > 0, "<stdint.h> INT_FAST8_WIDTH");
#endif
#ifndef INT_FAST16_WIDTH
static_assert(sizeof(INT_FAST16_WIDTH) > 0, "<stdint.h> INT_FAST16_WIDTH");
#endif
#ifndef INT_FAST32_WIDTH
static_assert(sizeof(INT_FAST32_WIDTH) > 0, "<stdint.h> INT_FAST32_WIDTH");
#endif
#ifndef INT_FAST64_WIDTH
static_assert(sizeof(INT_FAST64_WIDTH) > 0, "<stdint.h> INT_FAST64_WIDTH");
#endif
#ifndef SIZE_MAX
static_assert(sizeof(SIZE_MAX) > 0, "<stdint.h> SIZE_MAX");
#endif
#ifndef intptr_t
static_assert(sizeof(intptr_t) > 0, "<stdint.h> intptr_t");
#endif
#ifndef UINT_FAST8_MAX
static_assert(sizeof(UINT_FAST8_MAX) > 0, "<stdint.h> UINT_FAST8_MAX");
#endif
#ifndef UINT_FAST16_MAX
static_assert(sizeof(UINT_FAST16_MAX) > 0, "<stdint.h> UINT_FAST16_MAX");
#endif
#ifndef UINT_FAST32_MAX
static_assert(sizeof(UINT_FAST32_MAX) > 0, "<stdint.h> UINT_FAST32_MAX");
#endif
#ifndef UINT_FAST64_MAX
static_assert(sizeof(UINT_FAST64_MAX) > 0, "<stdint.h> UINT_FAST64_MAX");
#endif
#ifndef SIZE_WIDTH
static_assert(sizeof(SIZE_WIDTH) > 0, "<stdint.h> SIZE_WIDTH");
#endif
#ifndef uintptr_t
static_assert(sizeof(uintptr_t) > 0, "<stdint.h> uintptr_t");
#endif
#ifndef UINT_FAST8_WIDTH
static_assert(sizeof(UINT_FAST8_WIDTH) > 0, "<stdint.h> UINT_FAST8_WIDTH");
#endif
#ifndef UINT_FAST16_WIDTH
static_assert(sizeof(UINT_FAST16_WIDTH) > 0, "<stdint.h> UINT_FAST16_WIDTH");
#endif
#ifndef UINT_FAST32_WIDTH
static_assert(sizeof(UINT_FAST32_WIDTH) > 0, "<stdint.h> UINT_FAST32_WIDTH");
#endif
#ifndef UINT_FAST64_WIDTH
static_assert(sizeof(UINT_FAST64_WIDTH) > 0, "<stdint.h> UINT_FAST64_WIDTH");
#endif
#ifndef WCHAR_MIN
static_assert(sizeof(WCHAR_MIN) > 0, "<stdint.h> WCHAR_MIN");
#endif
#ifndef intmax_t
static_assert(sizeof(intmax_t) > 0, "<stdint.h> intmax_t");
#endif
#ifndef INTPTR_MIN
static_assert(sizeof(INTPTR_MIN) > 0, "<stdint.h> INTPTR_MIN");
#endif
#ifndef WCHAR_MAX
static_assert(sizeof(WCHAR_MAX) > 0, "<stdint.h> WCHAR_MAX");
#endif
#ifndef uintmax_t
static_assert(sizeof(uintmax_t) > 0, "<stdint.h> uintmax_t");
#endif
#ifndef INTPTR_MAX
static_assert(sizeof(INTPTR_MAX) > 0, "<stdint.h> INTPTR_MAX");
#endif
#ifndef WCHAR_WIDTH
static_assert(sizeof(WCHAR_WIDTH) > 0, "<stdint.h> WCHAR_WIDTH");
#endif
#ifndef __STDC_VERSION_STDINT_H__
static_assert(sizeof(__STDC_VERSION_STDINT_H__) > 0, "<stdint.h> __STDC_VERSION_STDINT_H__");
#endif
#ifndef INTPTR_WIDTH
static_assert(sizeof(INTPTR_WIDTH) > 0, "<stdint.h> INTPTR_WIDTH");
#endif
#ifndef WINT_MIN
static_assert(sizeof(WINT_MIN) > 0, "<stdint.h> WINT_MIN");
#endif
#ifndef INT8_MIN
static_assert(sizeof(INT8_MIN) > 0, "<stdint.h> INT8_MIN");
#endif
#ifndef INT16_MIN
static_assert(sizeof(INT16_MIN) > 0, "<stdint.h> INT16_MIN");
#endif
#ifndef INT32_MIN
static_assert(sizeof(INT32_MIN) > 0, "<stdint.h> INT32_MIN");
#endif
#ifndef INT64_MIN
static_assert(sizeof(INT64_MIN) > 0, "<stdint.h> INT64_MIN");
#endif
#ifndef UINTPTR_MAX
static_assert(sizeof(UINTPTR_MAX) > 0, "<stdint.h> UINTPTR_MAX");
#endif
#ifndef WINT_MAX
static_assert(sizeof(WINT_MAX) > 0, "<stdint.h> WINT_MAX");
#endif
#ifndef INT8_MAX
static_assert(sizeof(INT8_MAX) > 0, "<stdint.h> INT8_MAX");
#endif
#ifndef INT16_MAX
static_assert(sizeof(INT16_MAX) > 0, "<stdint.h> INT16_MAX");
#endif
#ifndef INT32_MAX
static_assert(sizeof(INT32_MAX) > 0, "<stdint.h> INT32_MAX");
#endif
#ifndef INT64_MAX
static_assert(sizeof(INT64_MAX) > 0, "<stdint.h> INT64_MAX");
#endif
#ifndef UINTPTR_WIDTH
static_assert(sizeof(UINTPTR_WIDTH) > 0, "<stdint.h> UINTPTR_WIDTH");
#endif
#ifndef WINT_WIDTH
static_assert(sizeof(WINT_WIDTH) > 0, "<stdint.h> WINT_WIDTH");
#endif
#ifndef size_t
static_assert(sizeof(size_t) > 0, "<stdio.h> size_t");
#endif
#ifndef _IONBF
static_assert(sizeof(_IONBF) > 0, "<stdio.h> _IONBF");
#endif
#ifndef SEEK_CUR
static_assert(sizeof(SEEK_CUR) > 0, "<stdio.h> SEEK_CUR");
#endif
#ifndef stdout
static_assert(sizeof(stdout) > 0, "<stdio.h> stdout");
#endif
static_assert(sizeof(FILE *) > 0, "<stdio.h> FILE");
#ifndef BUFSIZ
static_assert(sizeof(BUFSIZ) > 0, "<stdio.h> BUFSIZ");
#endif
#ifndef SEEK_END
static_assert(sizeof(SEEK_END) > 0, "<stdio.h> SEEK_END");
#endif
#ifndef _PRINTF_NAN_LEN_MAX
static_assert(sizeof(_PRINTF_NAN_LEN_MAX) > 0, "<stdio.h> _PRINTF_NAN_LEN_MAX");
#endif
#ifndef fpos_t
static_assert(sizeof(fpos_t) > 0, "<stdio.h> fpos_t");
#endif
#ifndef EOF
static_assert(sizeof(EOF) > 0, "<stdio.h> EOF");
#endif
#ifndef SEEK_SET
static_assert(sizeof(SEEK_SET) > 0, "<stdio.h> SEEK_SET");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<stdio.h> NULL");
#endif
#ifndef FOPEN_MAX
static_assert(sizeof(FOPEN_MAX) > 0, "<stdio.h> FOPEN_MAX");
#endif
#ifndef TMP_MAX
static_assert(sizeof(TMP_MAX) > 0, "<stdio.h> TMP_MAX");
#endif
#ifndef __STDC_VERSION_STDIO_H__
static_assert(sizeof(__STDC_VERSION_STDIO_H__) > 0, "<stdio.h> __STDC_VERSION_STDIO_H__");
#endif
#ifndef _IOFBF
static_assert(sizeof(_IOFBF) > 0, "<stdio.h> _IOFBF");
#endif
#ifndef FILENAME_MAX
static_assert(sizeof(FILENAME_MAX) > 0, "<stdio.h> FILENAME_MAX");
#endif
#ifndef stderr
static_assert(sizeof(stderr) > 0, "<stdio.h> stderr");
#endif
#ifndef _IOLBF
static_assert(sizeof(_IOLBF) > 0, "<stdio.h> _IOLBF");
#endif
#ifndef L_tmpnam
static_assert(sizeof(L_tmpnam) > 0, "<stdio.h> L_tmpnam");
#endif
#ifndef stdin
static_assert(sizeof(stdin) > 0, "<stdio.h> stdin");
#endif
#ifndef size_t
static_assert(sizeof(size_t) > 0, "<stdlib.h> size_t");
#endif
#ifndef once_flag
static_assert(sizeof(once_flag) > 0, "<stdlib.h> once_flag");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<stdlib.h> NULL");
#endif
#ifndef wchar_t
static_assert(sizeof(wchar_t) > 0, "<stdlib.h> wchar_t");
#endif
#ifndef __STDC_VERSION_STDLIB_H__
static_assert(sizeof(__STDC_VERSION_STDLIB_H__) > 0, "<stdlib.h> __STDC_VERSION_STDLIB_H__");
#endif
#ifndef ONCE_FLAG_INIT
static_assert(sizeof(ONCE_FLAG_INIT) > 0, "<stdlib.h> ONCE_FLAG_INIT");
#endif
#ifndef div_t
static_assert(sizeof(div_t) > 0, "<stdlib.h> div_t");
#endif
#ifndef EXIT_FAILURE
static_assert(sizeof(EXIT_FAILURE) > 0, "<stdlib.h> EXIT_FAILURE");
#endif
#ifndef RAND_MAX
static_assert(sizeof(RAND_MAX) > 0, "<stdlib.h> RAND_MAX");
#endif
#ifndef ldiv_t
static_assert(sizeof(ldiv_t) > 0, "<stdlib.h> ldiv_t");
#endif
#ifndef EXIT_SUCCESS
static_assert(sizeof(EXIT_SUCCESS) > 0, "<stdlib.h> EXIT_SUCCESS");
#endif
#ifndef lldiv_t
static_assert(sizeof(lldiv_t) > 0, "<stdlib.h> lldiv_t");
#endif
#ifndef MB_CUR_MAX
static_assert(sizeof(MB_CUR_MAX) > 0, "<stdlib.h> MB_CUR_MAX");
#endif
#ifndef noreturn
static_assert(sizeof(noreturn) > 0, "<stdnoreturn.h> noreturn");
#endif
#ifndef size_t
static_assert(sizeof(size_t) > 0, "<string.h> size_t");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<string.h> NULL");
#endif
#ifndef __STDC_VERSION_STRING_H__
static_assert(sizeof(__STDC_VERSION_STRING_H__) > 0, "<string.h> __STDC_VERSION_STRING_H__");
#endif
#ifndef __STDC_VERSION_TGMATH_H__
static_assert(sizeof(__STDC_VERSION_TGMATH_H__) > 0, "<tgmath.h> __STDC_VERSION_TGMATH_H__");
#endif
#ifndef sqrt
static_assert(sizeof(sqrt) > 0, "<tgmath.h> sqrt");
#endif
#ifndef exp2m1
static_assert(sizeof(exp2m1) > 0, "<tgmath.h> exp2m1");
#endif
#ifndef acos
static_assert(sizeof(acos) > 0, "<tgmath.h> acos");
#endif
#ifndef fabs
static_assert(sizeof(fabs) > 0, "<tgmath.h> fabs");
#endif
#ifndef exp2
static_assert(sizeof(exp2) > 0, "<tgmath.h> exp2");
#endif
#ifndef asin
static_assert(sizeof(asin) > 0, "<tgmath.h> asin");
#endif
#ifndef acospi
static_assert(sizeof(acospi) > 0, "<tgmath.h> acospi");
#endif
#ifndef expm1
static_assert(sizeof(expm1) > 0, "<tgmath.h> expm1");
#endif
#ifndef atan
static_assert(sizeof(atan) > 0, "<tgmath.h> atan");
#endif
#ifndef asinpi
static_assert(sizeof(asinpi) > 0, "<tgmath.h> asinpi");
#endif
#ifndef fdim
static_assert(sizeof(fdim) > 0, "<tgmath.h> fdim");
#endif
#ifndef acosh
static_assert(sizeof(acosh) > 0, "<tgmath.h> acosh");
#endif
#ifndef atan2pi
static_assert(sizeof(atan2pi) > 0, "<tgmath.h> atan2pi");
#endif
#ifndef floor
static_assert(sizeof(floor) > 0, "<tgmath.h> floor");
#endif
#ifndef asinh
static_assert(sizeof(asinh) > 0, "<tgmath.h> asinh");
#endif
#ifndef atan2
static_assert(sizeof(atan2) > 0, "<tgmath.h> atan2");
#endif
#ifndef fmax
static_assert(sizeof(fmax) > 0, "<tgmath.h> fmax");
#endif
#ifndef atanh
static_assert(sizeof(atanh) > 0, "<tgmath.h> atanh");
#endif
#ifndef atanpi
static_assert(sizeof(atanpi) > 0, "<tgmath.h> atanpi");
#endif
#ifndef fmaximum
static_assert(sizeof(fmaximum) > 0, "<tgmath.h> fmaximum");
#endif
#ifndef cos
static_assert(sizeof(cos) > 0, "<tgmath.h> cos");
#endif
#ifndef cbrt
static_assert(sizeof(cbrt) > 0, "<tgmath.h> cbrt");
#endif
#ifndef fmaximum_mag
static_assert(sizeof(fmaximum_mag) > 0, "<tgmath.h> fmaximum_mag");
#endif
#ifndef sin
static_assert(sizeof(sin) > 0, "<tgmath.h> sin");
#endif
#ifndef ceil
static_assert(sizeof(ceil) > 0, "<tgmath.h> ceil");
#endif
#ifndef fmaximum_num
static_assert(sizeof(fmaximum_num) > 0, "<tgmath.h> fmaximum_num");
#endif
#ifndef tan
static_assert(sizeof(tan) > 0, "<tgmath.h> tan");
#endif
#ifndef compoundn
static_assert(sizeof(compoundn) > 0, "<tgmath.h> compoundn");
#endif
#ifndef fmaximum_mag_num
static_assert(sizeof(fmaximum_mag_num) > 0, "<tgmath.h> fmaximum_mag_num");
#endif
#ifndef cosh
static_assert(sizeof(cosh) > 0, "<tgmath.h> cosh");
#endif
#ifndef copysign
static_assert(sizeof(copysign) > 0, "<tgmath.h> copysign");
#endif
#ifndef fma
static_assert(sizeof(fma) > 0, "<tgmath.h> fma");
#endif
#ifndef sinh
static_assert(sizeof(sinh) > 0, "<tgmath.h> sinh");
#endif
#ifndef cospi
static_assert(sizeof(cospi) > 0, "<tgmath.h> cospi");
#endif
#ifndef fmin
static_assert(sizeof(fmin) > 0, "<tgmath.h> fmin");
#endif
#ifndef tanh
static_assert(sizeof(tanh) > 0, "<tgmath.h> tanh");
#endif
#ifndef erfc
static_assert(sizeof(erfc) > 0, "<tgmath.h> erfc");
#endif
#ifndef fminimum
static_assert(sizeof(fminimum) > 0, "<tgmath.h> fminimum");
#endif
#ifndef exp
static_assert(sizeof(exp) > 0, "<tgmath.h> exp");
#endif
#ifndef erf
static_assert(sizeof(erf) > 0, "<tgmath.h> erf");
#endif
#ifndef fminimum_mag
static_assert(sizeof(fminimum_mag) > 0, "<tgmath.h> fminimum_mag");
#endif
#ifndef log
static_assert(sizeof(log) > 0, "<tgmath.h> log");
#endif
#ifndef exp10m1
static_assert(sizeof(exp10m1) > 0, "<tgmath.h> exp10m1");
#endif
#ifndef fminimum_num
static_assert(sizeof(fminimum_num) > 0, "<tgmath.h> fminimum_num");
#endif
#ifndef pow
static_assert(sizeof(pow) > 0, "<tgmath.h> pow");
#endif
#ifndef exp10
static_assert(sizeof(exp10) > 0, "<tgmath.h> exp10");
#endif
#ifndef fminimum_mag_num
static_assert(sizeof(fminimum_mag_num) > 0, "<tgmath.h> fminimum_mag_num");
#endif
#ifndef fmod
static_assert(sizeof(fmod) > 0, "<tgmath.h> fmod");
#endif
#ifndef lrint
static_assert(sizeof(lrint) > 0, "<tgmath.h> lrint");
#endif
#ifndef sinpi
static_assert(sizeof(sinpi) > 0, "<tgmath.h> sinpi");
#endif
#ifndef frexp
static_assert(sizeof(frexp) > 0, "<tgmath.h> frexp");
#endif
#ifndef lround
static_assert(sizeof(lround) > 0, "<tgmath.h> lround");
#endif
#ifndef tanpi
static_assert(sizeof(tanpi) > 0, "<tgmath.h> tanpi");
#endif
#ifndef fromfpx
static_assert(sizeof(fromfpx) > 0, "<tgmath.h> fromfpx");
#endif
#ifndef nearbyint
static_assert(sizeof(nearbyint) > 0, "<tgmath.h> nearbyint");
#endif
#ifndef tgamma
static_assert(sizeof(tgamma) > 0, "<tgmath.h> tgamma");
#endif
#ifndef fromfp
static_assert(sizeof(fromfp) > 0, "<tgmath.h> fromfp");
#endif
#ifndef nextafter
static_assert(sizeof(nextafter) > 0, "<tgmath.h> nextafter");
#endif
#ifndef trunc
static_assert(sizeof(trunc) > 0, "<tgmath.h> trunc");
#endif
#ifndef hypot
static_assert(sizeof(hypot) > 0, "<tgmath.h> hypot");
#endif
#ifndef nextdown
static_assert(sizeof(nextdown) > 0, "<tgmath.h> nextdown");
#endif
#ifndef ufromfpx
static_assert(sizeof(ufromfpx) > 0, "<tgmath.h> ufromfpx");
#endif
#ifndef ilogb
static_assert(sizeof(ilogb) > 0, "<tgmath.h> ilogb");
#endif
#ifndef nexttoward
static_assert(sizeof(nexttoward) > 0, "<tgmath.h> nexttoward");
#endif
#ifndef ufromfp
static_assert(sizeof(ufromfp) > 0, "<tgmath.h> ufromfp");
#endif
#ifndef ldexp
static_assert(sizeof(ldexp) > 0, "<tgmath.h> ldexp");
#endif
#ifndef nextup
static_assert(sizeof(nextup) > 0, "<tgmath.h> nextup");
#endif
#ifndef fadd
static_assert(sizeof(fadd) > 0, "<tgmath.h> fadd");
#endif
#ifndef lgamma
static_assert(sizeof(lgamma) > 0, "<tgmath.h> lgamma");
#endif
#ifndef pown
static_assert(sizeof(pown) > 0, "<tgmath.h> pown");
#endif
#ifndef dadd
static_assert(sizeof(dadd) > 0, "<tgmath.h> dadd");
#endif
#ifndef llogb
static_assert(sizeof(llogb) > 0, "<tgmath.h> llogb");
#endif
#ifndef powr
static_assert(sizeof(powr) > 0, "<tgmath.h> powr");
#endif
#ifndef fsub
static_assert(sizeof(fsub) > 0, "<tgmath.h> fsub");
#endif
#ifndef llrint
static_assert(sizeof(llrint) > 0, "<tgmath.h> llrint");
#endif
#ifndef remainder
static_assert(sizeof(remainder) > 0, "<tgmath.h> remainder");
#endif
#ifndef dsub
static_assert(sizeof(dsub) > 0, "<tgmath.h> dsub");
#endif
#ifndef llround
static_assert(sizeof(llround) > 0, "<tgmath.h> llround");
#endif
#ifndef remquo
static_assert(sizeof(remquo) > 0, "<tgmath.h> remquo");
#endif
#ifndef fmul
static_assert(sizeof(fmul) > 0, "<tgmath.h> fmul");
#endif
#ifndef log10p1
static_assert(sizeof(log10p1) > 0, "<tgmath.h> log10p1");
#endif
#ifndef rint
static_assert(sizeof(rint) > 0, "<tgmath.h> rint");
#endif
#ifndef dmul
static_assert(sizeof(dmul) > 0, "<tgmath.h> dmul");
#endif
#ifndef log10
static_assert(sizeof(log10) > 0, "<tgmath.h> log10");
#endif
#ifndef rootn
static_assert(sizeof(rootn) > 0, "<tgmath.h> rootn");
#endif
#ifndef fdiv
static_assert(sizeof(fdiv) > 0, "<tgmath.h> fdiv");
#endif
#ifndef log1p
static_assert(sizeof(log1p) > 0, "<tgmath.h> log1p");
#endif
#ifndef roundeven
static_assert(sizeof(roundeven) > 0, "<tgmath.h> roundeven");
#endif
#ifndef ddiv
static_assert(sizeof(ddiv) > 0, "<tgmath.h> ddiv");
#endif
#ifndef log2p1
static_assert(sizeof(log2p1) > 0, "<tgmath.h> log2p1");
#endif
#ifndef round
static_assert(sizeof(round) > 0, "<tgmath.h> round");
#endif
#ifndef ffma
static_assert(sizeof(ffma) > 0, "<tgmath.h> ffma");
#endif
#ifndef log2
static_assert(sizeof(log2) > 0, "<tgmath.h> log2");
#endif
#ifndef rsqrt
static_assert(sizeof(rsqrt) > 0, "<tgmath.h> rsqrt");
#endif
#ifndef dfma
static_assert(sizeof(dfma) > 0, "<tgmath.h> dfma");
#endif
#ifndef logb
static_assert(sizeof(logb) > 0, "<tgmath.h> logb");
#endif
#ifndef scalbln
static_assert(sizeof(scalbln) > 0, "<tgmath.h> scalbln");
#endif
#ifndef fsqrt
static_assert(sizeof(fsqrt) > 0, "<tgmath.h> fsqrt");
#endif
#ifndef logp1
static_assert(sizeof(logp1) > 0, "<tgmath.h> logp1");
#endif
#ifndef scalbn
static_assert(sizeof(scalbn) > 0, "<tgmath.h> scalbn");
#endif
#ifndef dsqrt
static_assert(sizeof(dsqrt) > 0, "<tgmath.h> dsqrt");
#endif
#ifndef carg
static_assert(sizeof(carg) > 0, "<tgmath.h> carg");
#endif
#ifndef cimag
static_assert(sizeof(cimag) > 0, "<tgmath.h> cimag");
#endif
#ifndef conj
static_assert(sizeof(conj) > 0, "<tgmath.h> conj");
#endif
#ifndef cproj
static_assert(sizeof(cproj) > 0, "<tgmath.h> cproj");
#endif
#ifndef creal
static_assert(sizeof(creal) > 0, "<tgmath.h> creal");
#endif
#ifndef mtx_t
static_assert(sizeof(mtx_t) > 0, "<threads.h> mtx_t");
#endif
#ifndef mtx_timed
static_assert(sizeof(mtx_timed) > 0, "<threads.h> mtx_timed");
#endif
#ifndef ONCE_FLAG_INIT
static_assert(sizeof(ONCE_FLAG_INIT) > 0, "<threads.h> ONCE_FLAG_INIT");
#endif
#ifndef tss_dtor_t
static_assert(sizeof(tss_dtor_t) > 0, "<threads.h> tss_dtor_t");
#endif
#ifndef thrd_timedout
static_assert(sizeof(thrd_timedout) > 0, "<threads.h> thrd_timedout");
#endif
#ifndef TSS_DTOR_ITERATIONS
static_assert(sizeof(TSS_DTOR_ITERATIONS) > 0, "<threads.h> TSS_DTOR_ITERATIONS");
#endif
#ifndef thrd_start_t
static_assert(sizeof(thrd_start_t) > 0, "<threads.h> thrd_start_t");
#endif
#ifndef thrd_success
static_assert(sizeof(thrd_success) > 0, "<threads.h> thrd_success");
#endif
#ifndef cnd_t
static_assert(sizeof(cnd_t) > 0, "<threads.h> cnd_t");
#endif
#ifndef once_flag
static_assert(sizeof(once_flag) > 0, "<threads.h> once_flag");
#endif
#ifndef thrd_busy
static_assert(sizeof(thrd_busy) > 0, "<threads.h> thrd_busy");
#endif
#ifndef thrd_t
static_assert(sizeof(thrd_t) > 0, "<threads.h> thrd_t");
#endif
#ifndef mtx_plain
static_assert(sizeof(mtx_plain) > 0, "<threads.h> mtx_plain");
#endif
#ifndef thrd_error
static_assert(sizeof(thrd_error) > 0, "<threads.h> thrd_error");
#endif
#ifndef tss_t
static_assert(sizeof(tss_t) > 0, "<threads.h> tss_t");
#endif
#ifndef mtx_recursive
static_assert(sizeof(mtx_recursive) > 0, "<threads.h> mtx_recursive");
#endif
#ifndef thrd_nomem
static_assert(sizeof(thrd_nomem) > 0, "<threads.h> thrd_nomem");
#endif
#ifndef __STDC_VERSION_TIME_H__
static_assert(sizeof(__STDC_VERSION_TIME_H__) > 0, "<time.h> __STDC_VERSION_TIME_H__");
#endif
#ifndef TIME_UTC
static_assert(sizeof(TIME_UTC) > 0, "<time.h> TIME_UTC");
#endif
#ifndef time_t
static_assert(sizeof(time_t) > 0, "<time.h> time_t");
#endif
#ifndef NULL
static_assert(sizeof(NULL) > 0, "<time.h> NULL");
#endif
#ifndef size_t
static_assert(sizeof(size_t) > 0, "<time.h> size_t");
#endif
static_assert(sizeof(struct timespec) > 0, "<time.h> struct timespec");
#ifndef CLOCKS_PER_SEC
static_assert(sizeof(CLOCKS_PER_SEC) > 0, "<time.h> CLOCKS_PER_SEC");
#endif
#ifndef clock_t
static_assert(sizeof(clock_t) > 0, "<time.h> clock_t");
#endif
static_assert(sizeof(struct tm) > 0, "<time.h> struct tm");

/* Every function, redeclared with its Annex B type (a mismatch does not compile); the parentheses bypass masking macros */
double complex(cacos)(double complex z);
float complex(cacosf)(float complex z);
long double complex(cacosl)(long double complex z);
double complex(casin)(double complex z);
float complex(casinf)(float complex z);
long double complex(casinl)(long double complex z);
double complex(catan)(double complex z);
float complex(catanf)(float complex z);
long double complex(catanl)(long double complex z);
double complex(ccos)(double complex z);
float complex(ccosf)(float complex z);
long double complex(ccosl)(long double complex z);
double complex(csin)(double complex z);
float complex(csinf)(float complex z);
long double complex(csinl)(long double complex z);
double complex(ctan)(double complex z);
float complex(ctanf)(float complex z);
long double complex(ctanl)(long double complex z);
double complex(cacosh)(double complex z);
float complex(cacoshf)(float complex z);
long double complex(cacoshl)(long double complex z);
double complex(casinh)(double complex z);
float complex(casinhf)(float complex z);
long double complex(casinhl)(long double complex z);
double complex(catanh)(double complex z);
float complex(catanhf)(float complex z);
long double complex(catanhl)(long double complex z);
double complex(ccosh)(double complex z);
float complex(ccoshf)(float complex z);
long double complex(ccoshl)(long double complex z);
double complex(csinh)(double complex z);
float complex(csinhf)(float complex z);
long double complex(csinhl)(long double complex z);
double complex(ctanh)(double complex z);
float complex(ctanhf)(float complex z);
long double complex(ctanhl)(long double complex z);
double complex(cexp)(double complex z);
float complex(cexpf)(float complex z);
long double complex(cexpl)(long double complex z);
double complex(clog)(double complex z);
float complex(clogf)(float complex z);
long double complex(clogl)(long double complex z);
double(cabs)(double complex z);
float(cabsf)(float complex z);
long double(cabsl)(long double complex z);
double complex(cpow)(double complex x, double complex y);
float complex(cpowf)(float complex x, float complex y);
long double complex(cpowl)(long double complex x, long double complex y);
double complex(csqrt)(double complex z);
float complex(csqrtf)(float complex z);
long double complex(csqrtl)(long double complex z);
double(carg)(double complex z);
float(cargf)(float complex z);
long double(cargl)(long double complex z);
double(cimag)(double complex z);
float(cimagf)(float complex z);
long double(cimagl)(long double complex z);
double complex(conj)(double complex z);
float complex(conjf)(float complex z);
long double complex(conjl)(long double complex z);
double complex(cproj)(double complex z);
float complex(cprojf)(float complex z);
long double complex(cprojl)(long double complex z);
double(creal)(double complex z);
float(crealf)(float complex z);
long double(creall)(long double complex z);
int(isalnum)(int c);
int(isalpha)(int c);
int(isblank)(int c);
int(iscntrl)(int c);
int(isdigit)(int c);
int(isgraph)(int c);
int(islower)(int c);
int(isprint)(int c);
int(ispunct)(int c);
int(isspace)(int c);
int(isupper)(int c);
int(isxdigit)(int c);
int(tolower)(int c);
int(toupper)(int c);
int(feclearexcept)(int excepts);
int(fegetexceptflag)(fexcept_t *flagp, int excepts);
int(feraiseexcept)(int excepts);
int(fesetexcept)(int excepts);
int(fesetexceptflag)(const fexcept_t *flagp, int excepts);
int(fetestexceptflag)(const fexcept_t *flagp, int excepts);
int(fetestexcept)(int excepts);
int(fegetmode)(femode_t *modep);
int(fegetround)(void);
int(fesetmode)(const femode_t *modep);
int(fesetround)(int rnd);
int(fegetenv)(fenv_t *envp);
int(feholdexcept)(fenv_t *envp);
int(fesetenv)(const fenv_t *envp);
int(feupdateenv)(const fenv_t *envp);
int(fe_dec_getround)(void);
int(fe_dec_setround)(int rnd);
intmax_t(imaxabs)(intmax_t j);
imaxdiv_t(imaxdiv)(intmax_t numer, intmax_t denom);
intmax_t(strtoimax)(const char *restrict nptr, char **restrict endptr, int base);
uintmax_t(strtoumax)(const char *restrict nptr, char **restrict endptr, int base);
intmax_t(wcstoimax)(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
uintmax_t(wcstoumax)(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
char *(setlocale)(int category, const char *locale);
struct lconv *(localeconv)(void);
double(acos)(double x);
float(acosf)(float x);
long double(acosl)(long double x);
double(asin)(double x);
float(asinf)(float x);
long double(asinl)(long double x);
double(atan)(double x);
float(atanf)(float x);
long double(atanl)(long double x);
double(atan2)(double y, double x);
float(atan2f)(float y, float x);
long double(atan2l)(long double y, long double x);
double(cos)(double x);
float(cosf)(float x);
long double(cosl)(long double x);
double(sin)(double x);
float(sinf)(float x);
long double(sinl)(long double x);
double(tan)(double x);
float(tanf)(float x);
long double(tanl)(long double x);
double(acospi)(double x);
float(acospif)(float x);
long double(acospil)(long double x);
double(asinpi)(double x);
float(asinpif)(float x);
long double(asinpil)(long double x);
double(atanpi)(double x);
float(atanpif)(float x);
long double(atanpil)(long double x);
double(atan2pi)(double y, double x);
float(atan2pif)(float y, float x);
long double(atan2pil)(long double y, long double x);
double(cospi)(double x);
float(cospif)(float x);
long double(cospil)(long double x);
double(sinpi)(double x);
float(sinpif)(float x);
long double(sinpil)(long double x);
double(tanpi)(double x);
float(tanpif)(float x);
long double(tanpil)(long double x);
double(acosh)(double x);
float(acoshf)(float x);
long double(acoshl)(long double x);
double(asinh)(double x);
float(asinhf)(float x);
long double(asinhl)(long double x);
double(atanh)(double x);
float(atanhf)(float x);
long double(atanhl)(long double x);
double(cosh)(double x);
float(coshf)(float x);
long double(coshl)(long double x);
double(sinh)(double x);
float(sinhf)(float x);
long double(sinhl)(long double x);
double(tanh)(double x);
float(tanhf)(float x);
long double(tanhl)(long double x);
double(exp)(double x);
float(expf)(float x);
long double(expl)(long double x);
double(exp10)(double x);
float(exp10f)(float x);
long double(exp10l)(long double x);
double(exp10m1)(double x);
float(exp10m1f)(float x);
long double(exp10m1l)(long double x);
double(exp2)(double x);
float(exp2f)(float x);
long double(exp2l)(long double x);
double(exp2m1)(double x);
float(exp2m1f)(float x);
long double(exp2m1l)(long double x);
double(expm1)(double x);
float(expm1f)(float x);
long double(expm1l)(long double x);
double(frexp)(double value, int *p);
float(frexpf)(float value, int *p);
long double(frexpl)(long double value, int *p);
int(ilogb)(double x);
int(ilogbf)(float x);
int(ilogbl)(long double x);
double(ldexp)(double x, int p);
float(ldexpf)(float x, int p);
long double(ldexpl)(long double x, int p);
long int(llogb)(double x);
long int(llogbf)(float x);
long int(llogbl)(long double x);
double(log)(double x);
float(logf)(float x);
long double(logl)(long double x);
double(log10)(double x);
float(log10f)(float x);
long double(log10l)(long double x);
double(log10p1)(double x);
float(log10p1f)(float x);
long double(log10p1l)(long double x);
double(log1p)(double x);
float(log1pf)(float x);
long double(log1pl)(long double x);
double(logp1)(double x);
float(logp1f)(float x);
long double(logp1l)(long double x);
double(log2)(double x);
float(log2f)(float x);
long double(log2l)(long double x);
double(log2p1)(double x);
float(log2p1f)(float x);
long double(log2p1l)(long double x);
double(logb)(double x);
float(logbf)(float x);
long double(logbl)(long double x);
double(modf)(double value, double *iptr);
float(modff)(float value, float *iptr);
long double(modfl)(long double value, long double *iptr);
double(scalbn)(double x, int n);
float(scalbnf)(float x, int n);
long double(scalbnl)(long double x, int n);
double(scalbln)(double x, long int n);
float(scalblnf)(float x, long int n);
long double(scalblnl)(long double x, long int n);
double(cbrt)(double x);
float(cbrtf)(float x);
long double(cbrtl)(long double x);
double(compoundn)(double x, long long int n);
float(compoundnf)(float x, long long int n);
long double(compoundnl)(long double x, long long int n);
double(fabs)(double x);
float(fabsf)(float x);
long double(fabsl)(long double x);
double(hypot)(double x, double y);
float(hypotf)(float x, float y);
long double(hypotl)(long double x, long double y);
double(pow)(double x, double y);
float(powf)(float x, float y);
long double(powl)(long double x, long double y);
double(pown)(double x, long long int n);
float(pownf)(float x, long long int n);
long double(pownl)(long double x, long long int n);
double(powr)(double y, double x);
float(powrf)(float y, float x);
long double(powrl)(long double y, long double x);
double(rootn)(double x, long long int n);
float(rootnf)(float x, long long int n);
long double(rootnl)(long double x, long long int n);
double(rsqrt)(double x);
float(rsqrtf)(float x);
long double(rsqrtl)(long double x);
double(sqrt)(double x);
float(sqrtf)(float x);
long double(sqrtl)(long double x);
double(erf)(double x);
float(erff)(float x);
long double(erfl)(long double x);
double(erfc)(double x);
float(erfcf)(float x);
long double(erfcl)(long double x);
double(lgamma)(double x);
float(lgammaf)(float x);
long double(lgammal)(long double x);
double(tgamma)(double x);
float(tgammaf)(float x);
long double(tgammal)(long double x);
double(ceil)(double x);
float(ceilf)(float x);
long double(ceill)(long double x);
double(floor)(double x);
float(floorf)(float x);
long double(floorl)(long double x);
double(nearbyint)(double x);
float(nearbyintf)(float x);
long double(nearbyintl)(long double x);
double(rint)(double x);
float(rintf)(float x);
long double(rintl)(long double x);
long int(lrint)(double x);
long int(lrintf)(float x);
long int(lrintl)(long double x);
long long int(llrint)(double x);
long long int(llrintf)(float x);
long long int(llrintl)(long double x);
double(round)(double x);
float(roundf)(float x);
long double(roundl)(long double x);
long int(lround)(double x);
long int(lroundf)(float x);
long int(lroundl)(long double x);
long long int(llround)(double x);
long long int(llroundf)(float x);
long long int(llroundl)(long double x);
double(roundeven)(double x);
float(roundevenf)(float x);
long double(roundevenl)(long double x);
double(trunc)(double x);
float(truncf)(float x);
long double(truncl)(long double x);
double(fromfp)(double x, int rnd, unsigned int width);
float(fromfpf)(float x, int rnd, unsigned int width);
long double(fromfpl)(long double x, int rnd, unsigned int width);
double(ufromfp)(double x, int rnd, unsigned int width);
float(ufromfpf)(float x, int rnd, unsigned int width);
long double(ufromfpl)(long double x, int rnd, unsigned int width);
double(fromfpx)(double x, int rnd, unsigned int width);
float(fromfpxf)(float x, int rnd, unsigned int width);
long double(fromfpxl)(long double x, int rnd, unsigned int width);
double(ufromfpx)(double x, int rnd, unsigned int width);
float(ufromfpxf)(float x, int rnd, unsigned int width);
long double(ufromfpxl)(long double x, int rnd, unsigned int width);
double(fmod)(double x, double y);
float(fmodf)(float x, float y);
long double(fmodl)(long double x, long double y);
double(remainder)(double x, double y);
float(remainderf)(float x, float y);
long double(remainderl)(long double x, long double y);
double(remquo)(double x, double y, int *quo);
float(remquof)(float x, float y, int *quo);
long double(remquol)(long double x, long double y, int *quo);
double(copysign)(double x, double y);
float(copysignf)(float x, float y);
long double(copysignl)(long double x, long double y);
double(nan)(const char *tagp);
float(nanf)(const char *tagp);
long double(nanl)(const char *tagp);
double(nextafter)(double x, double y);
float(nextafterf)(float x, float y);
long double(nextafterl)(long double x, long double y);
double(nexttoward)(double x, long double y);
float(nexttowardf)(float x, long double y);
long double(nexttowardl)(long double x, long double y);
double(nextup)(double x);
float(nextupf)(float x);
long double(nextupl)(long double x);
double(nextdown)(double x);
float(nextdownf)(float x);
long double(nextdownl)(long double x);
int(canonicalize)(double *cx, const double *x);
int(canonicalizef)(float *cx, const float *x);
int(canonicalizel)(long double *cx, const long double *x);
double(fdim)(double x, double y);
float(fdimf)(float x, float y);
long double(fdiml)(long double x, long double y);
double(fmax)(double x, double y);
float(fmaxf)(float x, float y);
long double(fmaxl)(long double x, long double y);
double(fmin)(double x, double y);
float(fminf)(float x, float y);
long double(fminl)(long double x, long double y);
double(fmaximum)(double x, double y);
float(fmaximumf)(float x, float y);
long double(fmaximuml)(long double x, long double y);
double(fminimum)(double x, double y);
float(fminimumf)(float x, float y);
long double(fminimuml)(long double x, long double y);
double(fmaximum_mag)(double x, double y);
float(fmaximum_magf)(float x, float y);
long double(fmaximum_magl)(long double x, long double y);
double(fminimum_mag)(double x, double y);
float(fminimum_magf)(float x, float y);
long double(fminimum_magl)(long double x, long double y);
double(fmaximum_num)(double x, double y);
float(fmaximum_numf)(float x, float y);
long double(fmaximum_numl)(long double x, long double y);
double(fminimum_num)(double x, double y);
float(fminimum_numf)(float x, float y);
long double(fminimum_numl)(long double x, long double y);
double(fmaximum_mag_num)(double x, double y);
float(fmaximum_mag_numf)(float x, float y);
long double(fmaximum_mag_numl)(long double x, long double y);
double(fminimum_mag_num)(double x, double y);
float(fminimum_mag_numf)(float x, float y);
long double(fminimum_mag_numl)(long double x, long double y);
double(fma)(double x, double y, double z);
float(fmaf)(float x, float y, float z);
long double(fmal)(long double x, long double y, long double z);
float(fadd)(double x, double y);
float(faddl)(long double x, long double y);
double(daddl)(long double x, long double y);
float(fsub)(double x, double y);
float(fsubl)(long double x, long double y);
double(dsubl)(long double x, long double y);
float(fmul)(double x, double y);
float(fmull)(long double x, long double y);
double(dmull)(long double x, long double y);
float(fdiv)(double x, double y);
float(fdivl)(long double x, long double y);
double(ddivl)(long double x, long double y);
float(ffma)(double x, double y, double z);
float(ffmal)(long double x, long double y, long double z);
double(dfmal)(long double x, long double y, long double z);
float(fsqrt)(double x);
float(fsqrtl)(long double x);
double(dsqrtl)(long double x);
int(totalorder)(const double *x, const double *y);
int(totalorderf)(const float *x, const float *y);
int(totalorderl)(const long double *x, const long double *y);
int(totalordermag)(const double *x, const double *y);
int(totalordermagf)(const float *x, const float *y);
int(totalordermagl)(const long double *x, const long double *y);
double(getpayload)(const double *x);
float(getpayloadf)(const float *x);
long double(getpayloadl)(const long double *x);
int(setpayload)(double *res, double pl);
int(setpayloadf)(float *res, float pl);
int(setpayloadl)(long double *res, long double pl);
int(setpayloadsig)(double *res, double pl);
int(setpayloadsigf)(float *res, float pl);
int(setpayloadsigl)(long double *res, long double pl);
void(longjmp)(jmp_buf env, int val);
void (*(signal)(int sig, void (*func)(int)))(int);
int(raise)(int sig);
void(atomic_thread_fence)(memory_order order);
void(atomic_signal_fence)(memory_order order);
bool(atomic_flag_test_and_set)(volatile atomic_flag *object);
bool(atomic_flag_test_and_set_explicit)(volatile atomic_flag *object, memory_order order);
void(atomic_flag_clear)(volatile atomic_flag *object);
void(atomic_flag_clear_explicit)(volatile atomic_flag *object, memory_order order);
unsigned int(stdc_leading_zeros_uc)(unsigned char value);
unsigned int(stdc_leading_zeros_us)(unsigned short value);
unsigned int(stdc_leading_zeros_ui)(unsigned int value);
unsigned int(stdc_leading_zeros_ul)(unsigned long int value);
unsigned int(stdc_leading_zeros_ull)(unsigned long long int value);
unsigned int(stdc_leading_ones_uc)(unsigned char value);
unsigned int(stdc_leading_ones_us)(unsigned short value);
unsigned int(stdc_leading_ones_ui)(unsigned int value);
unsigned int(stdc_leading_ones_ul)(unsigned long int value);
unsigned int(stdc_leading_ones_ull)(unsigned long long int value);
unsigned int(stdc_trailing_zeros_uc)(unsigned char value);
unsigned int(stdc_trailing_zeros_us)(unsigned short value);
unsigned int(stdc_trailing_zeros_ui)(unsigned int value);
unsigned int(stdc_trailing_zeros_ul)(unsigned long int value);
unsigned int(stdc_trailing_zeros_ull)(unsigned long long int value);
unsigned int(stdc_trailing_ones_uc)(unsigned char value);
unsigned int(stdc_trailing_ones_us)(unsigned short value);
unsigned int(stdc_trailing_ones_ui)(unsigned int value);
unsigned int(stdc_trailing_ones_ul)(unsigned long int value);
unsigned int(stdc_trailing_ones_ull)(unsigned long long int value);
unsigned int(stdc_first_leading_zero_uc)(unsigned char value);
unsigned int(stdc_first_leading_zero_us)(unsigned short value);
unsigned int(stdc_first_leading_zero_ui)(unsigned int value);
unsigned int(stdc_first_leading_zero_ul)(unsigned long int value);
unsigned int(stdc_first_leading_zero_ull)(unsigned long long int value);
unsigned int(stdc_first_leading_one_uc)(unsigned char value);
unsigned int(stdc_first_leading_one_us)(unsigned short value);
unsigned int(stdc_first_leading_one_ui)(unsigned int value);
unsigned int(stdc_first_leading_one_ul)(unsigned long int value);
unsigned int(stdc_first_leading_one_ull)(unsigned long long int value);
unsigned int(stdc_first_trailing_zero_uc)(unsigned char value);
unsigned int(stdc_first_trailing_zero_us)(unsigned short value);
unsigned int(stdc_first_trailing_zero_ui)(unsigned int value);
unsigned int(stdc_first_trailing_zero_ul)(unsigned long int value);
unsigned int(stdc_first_trailing_zero_ull)(unsigned long long int value);
unsigned int(stdc_first_trailing_one_uc)(unsigned char value);
unsigned int(stdc_first_trailing_one_us)(unsigned short value);
unsigned int(stdc_first_trailing_one_ui)(unsigned int value);
unsigned int(stdc_first_trailing_one_ul)(unsigned long int value);
unsigned int(stdc_first_trailing_one_ull)(unsigned long long int value);
unsigned int(stdc_count_zeros_uc)(unsigned char value);
unsigned int(stdc_count_zeros_us)(unsigned short value);
unsigned int(stdc_count_zeros_ui)(unsigned int value);
unsigned int(stdc_count_zeros_ul)(unsigned long int value);
unsigned int(stdc_count_zeros_ull)(unsigned long long int value);
unsigned int(stdc_count_ones_uc)(unsigned char value);
unsigned int(stdc_count_ones_us)(unsigned short value);
unsigned int(stdc_count_ones_ui)(unsigned int value);
unsigned int(stdc_count_ones_ul)(unsigned long int value);
unsigned int(stdc_count_ones_ull)(unsigned long long int value);
bool(stdc_has_single_bit_uc)(unsigned char value);
bool(stdc_has_single_bit_us)(unsigned short value);
bool(stdc_has_single_bit_ui)(unsigned int value);
bool(stdc_has_single_bit_ul)(unsigned long int value);
bool(stdc_has_single_bit_ull)(unsigned long long int value);
unsigned int(stdc_bit_width_uc)(unsigned char value);
unsigned int(stdc_bit_width_us)(unsigned short value);
unsigned int(stdc_bit_width_ui)(unsigned int value);
unsigned int(stdc_bit_width_ul)(unsigned long int value);
unsigned int(stdc_bit_width_ull)(unsigned long long int value);
unsigned char(stdc_bit_floor_uc)(unsigned char value);
unsigned short(stdc_bit_floor_us)(unsigned short value);
unsigned int(stdc_bit_floor_ui)(unsigned int value);
unsigned long int(stdc_bit_floor_ul)(unsigned long int value);
unsigned long long int(stdc_bit_floor_ull)(unsigned long long int value);
unsigned char(stdc_bit_ceil_uc)(unsigned char value);
unsigned short(stdc_bit_ceil_us)(unsigned short value);
unsigned int(stdc_bit_ceil_ui)(unsigned int value);
unsigned long int(stdc_bit_ceil_ul)(unsigned long int value);
unsigned long long int(stdc_bit_ceil_ull)(unsigned long long int value);
int(remove)(const char *filename);
int(rename)(const char *old, const char *new);
FILE *(tmpfile)(void);
char *(tmpnam)(char *s);
int(fclose)(FILE *stream);
int(fflush)(FILE *stream);
FILE *(fopen)(const char *restrict filename, const char *restrict mode);
FILE *(freopen)(const char *restrict filename, const char *restrict mode, FILE *restrict stream);
void(setbuf)(FILE *restrict stream, char *restrict buf);
int(setvbuf)(FILE *restrict stream, char *restrict buf, int mode, size_t size);
int(printf)(const char *restrict format, ...);
int(scanf)(const char *restrict format, ...);
int(snprintf)(char *restrict s, size_t n, const char *restrict format, ...);
int(sprintf)(char *restrict s, const char *restrict format, ...);
int(sscanf)(const char *restrict s, const char *restrict format, ...);
int(vfprintf)(FILE *restrict stream, const char *restrict format, va_list arg);
int(vfscanf)(FILE *restrict stream, const char *restrict format, va_list arg);
int(vprintf)(const char *restrict format, va_list arg);
int(vscanf)(const char *restrict format, va_list arg);
int(vsnprintf)(char *restrict s, size_t n, const char *restrict format, va_list arg);
int(vsprintf)(char *restrict s, const char *restrict format, va_list arg);
int(vsscanf)(const char *restrict s, const char *restrict format, va_list arg);
int(fgetc)(FILE *stream);
char *(fgets)(char *restrict s, int n, FILE *restrict stream);
int(fputc)(int c, FILE *stream);
int(fputs)(const char *restrict s, FILE *restrict stream);
int(getc)(FILE *stream);
int(getchar)(void);
int(putc)(int c, FILE *stream);
int(putchar)(int c);
int(puts)(const char *s);
int(ungetc)(int c, FILE *stream);
size_t(fread)(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t(fwrite)(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
int(fgetpos)(FILE *restrict stream, fpos_t *restrict pos);
int(fseek)(FILE *stream, long int offset, int whence);
int(fsetpos)(FILE *stream, const fpos_t *pos);
long int(ftell)(FILE *stream);
void(rewind)(FILE *stream);
void(clearerr)(FILE *stream);
int(feof)(FILE *stream);
int(ferror)(FILE *stream);
void(perror)(const char *s);
int(fprintf)(FILE *restrict stream, const char *restrict format, ...);
int(fscanf)(FILE *restrict stream, const char *restrict format, ...);
void(call_once)(once_flag *flag, void (*func)(void));
double(atof)(const char *nptr);
int(atoi)(const char *nptr);
long int(atol)(const char *nptr);
long long int(atoll)(const char *nptr);
int(strfromd)(char *restrict s, size_t n, const char *restrict format, double fp);
int(strfromf)(char *restrict s, size_t n, const char *restrict format, float fp);
int(strfroml)(char *restrict s, size_t n, const char *restrict format, long double fp);
double(strtod)(const char *restrict nptr, char **restrict endptr);
float(strtof)(const char *restrict nptr, char **restrict endptr);
long double(strtold)(const char *restrict nptr, char **restrict endptr);
long int(strtol)(const char *restrict nptr, char **restrict endptr, int base);
long long int(strtoll)(const char *restrict nptr, char **restrict endptr, int base);
unsigned long int(strtoul)(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long int(strtoull)(const char *restrict nptr, char **restrict endptr, int base);
int(rand)(void);
void(srand)(unsigned int seed);
void *(aligned_alloc)(size_t alignment, size_t size);
void *(calloc)(size_t nmemb, size_t size);
void(free)(void *ptr);
void(free_sized)(void *ptr, size_t size);
void(free_aligned_sized)(void *ptr, size_t alignment, size_t size);
void *(malloc)(size_t size);
void *(realloc)(void *ptr, size_t size);
void(abort)(void);
int(atexit)(void (*func)(void));
int(at_quick_exit)(void (*func)(void));
void(exit)(int status);
void(_Exit)(int status);
char *(getenv)(const char *name);
void(quick_exit)(int status);
int(system)(const char *string);
void(qsort)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
int(abs)(int j);
long int(labs)(long int j);
long long int(llabs)(long long int j);
div_t(div)(int numer, int denom);
ldiv_t(ldiv)(long int numer, long int denom);
lldiv_t(lldiv)(long long int numer, long long int denom);
int(mblen)(const char *s, size_t n);
int(mbtowc)(wchar_t *restrict pwc, const char *restrict s, size_t n);
int(wctomb)(char *s, wchar_t wc);
size_t(mbstowcs)(wchar_t *restrict pwcs, const char *restrict s, size_t n);
size_t(wcstombs)(char *restrict s, const wchar_t *restrict pwcs, size_t n);
size_t(memalignment)(const void *p);
void *(memcpy)(void *restrict s1, const void *restrict s2, size_t n);
void *(memccpy)(void *restrict s1, const void *restrict s2, int c, size_t n);
void *(memmove)(void *s1, const void *s2, size_t n);
char *(strcpy)(char *restrict s1, const char *restrict s2);
char *(strncpy)(char *restrict s1, const char *restrict s2, size_t n);
char *(strdup)(const char *s);
char *(strndup)(const char *s, size_t n);
char *(strcat)(char *restrict s1, const char *restrict s2);
char *(strncat)(char *restrict s1, const char *restrict s2, size_t n);
int(memcmp)(const void *s1, const void *s2, size_t n);
int(strcmp)(const char *s1, const char *s2);
int(strcoll)(const char *s1, const char *s2);
int(strncmp)(const char *s1, const char *s2, size_t n);
size_t(strxfrm)(char *restrict s1, const char *restrict s2, size_t n);
size_t(strcspn)(const char *s1, const char *s2);
size_t(strspn)(const char *s1, const char *s2);
char *(strtok)(char *restrict s1, const char *restrict s2);
void *(memset)(void *s, int c, size_t n);
void *(memset_explicit)(void *s, int c, size_t n);
char *(strerror)(int errnum);
size_t(strlen)(const char *s);
void(call_once)(once_flag *flag, void (*func)(void));
int(cnd_broadcast)(cnd_t *cond);
void(cnd_destroy)(cnd_t *cond);
int(cnd_init)(cnd_t *cond);
int(cnd_signal)(cnd_t *cond);
int(cnd_timedwait)(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts);
int(cnd_wait)(cnd_t *cond, mtx_t *mtx);
void(mtx_destroy)(mtx_t *mtx);
int(mtx_lock)(mtx_t *mtx);
int(mtx_timedlock)(mtx_t *restrict mtx, const struct timespec *restrict ts);
int(mtx_trylock)(mtx_t *mtx);
int(mtx_unlock)(mtx_t *mtx);
int(thrd_create)(thrd_t *thr, thrd_start_t func, void *arg);
thrd_t(thrd_current)(void);
int(thrd_detach)(thrd_t thr);
int(thrd_equal)(thrd_t thr0, thrd_t thr1);
void(thrd_exit)(int res);
int(thrd_join)(thrd_t thr, int *res);
int(thrd_sleep)(const struct timespec *duration, struct timespec *remaining);
void(thrd_yield)(void);
int(tss_create)(tss_t *key, tss_dtor_t dtor);
void(tss_delete)(tss_t key);
void *(tss_get)(tss_t key);
int(tss_set)(tss_t key, void *val);
clock_t(clock)(void);
double(difftime)(time_t time1, time_t time0);
time_t(mktime)(struct tm *timeptr);
time_t(timegm)(struct tm *timeptr);
time_t(time)(time_t *timer);
int(timespec_get)(struct timespec *ts, int base);
int(timespec_getres)(struct timespec *ts, int base);
char *(asctime)(const struct tm *timeptr);
char *(ctime)(const time_t *timer);
struct tm *(gmtime)(const time_t *timer);
struct tm *(gmtime_r)(const time_t *timer, struct tm *buf);
struct tm *(localtime)(const time_t *timer);
struct tm *(localtime_r)(const time_t *timer, struct tm *buf);
size_t(strftime)(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);

/* The generic functions and function-like macros (placeholder types in Annex B): defined as macros, or else declared */
#ifndef assert
static_assert(sizeof(&(assert)) > 0, "assert");
#endif
#ifndef CMPLX
static_assert(sizeof(&(CMPLX)) > 0, "CMPLX");
#endif
#ifndef CMPLXF
static_assert(sizeof(&(CMPLXF)) > 0, "CMPLXF");
#endif
#ifndef CMPLXL
static_assert(sizeof(&(CMPLXL)) > 0, "CMPLXL");
#endif
#ifndef fpclassify
static_assert(sizeof(&(fpclassify)) > 0, "fpclassify");
#endif
#ifndef iscanonical
static_assert(sizeof(&(iscanonical)) > 0, "iscanonical");
#endif
#ifndef isfinite
static_assert(sizeof(&(isfinite)) > 0, "isfinite");
#endif
#ifndef isinf
static_assert(sizeof(&(isinf)) > 0, "isinf");
#endif
#ifndef isnan
static_assert(sizeof(&(isnan)) > 0, "isnan");
#endif
#ifndef isnormal
static_assert(sizeof(&(isnormal)) > 0, "isnormal");
#endif
#ifndef signbit
static_assert(sizeof(&(signbit)) > 0, "signbit");
#endif
#ifndef issignaling
static_assert(sizeof(&(issignaling)) > 0, "issignaling");
#endif
#ifndef issubnormal
static_assert(sizeof(&(issubnormal)) > 0, "issubnormal");
#endif
#ifndef iszero
static_assert(sizeof(&(iszero)) > 0, "iszero");
#endif
#ifndef isgreater
static_assert(sizeof(&(isgreater)) > 0, "isgreater");
#endif
#ifndef isgreaterequal
static_assert(sizeof(&(isgreaterequal)) > 0, "isgreaterequal");
#endif
#ifndef isless
static_assert(sizeof(&(isless)) > 0, "isless");
#endif
#ifndef islessequal
static_assert(sizeof(&(islessequal)) > 0, "islessequal");
#endif
#ifndef islessgreater
static_assert(sizeof(&(islessgreater)) > 0, "islessgreater");
#endif
#ifndef isunordered
static_assert(sizeof(&(isunordered)) > 0, "isunordered");
#endif
#ifndef iseqsig
static_assert(sizeof(&(iseqsig)) > 0, "iseqsig");
#endif
#ifndef setjmp
static_assert(sizeof(&(setjmp)) > 0, "setjmp");
#endif
#ifndef va_arg
static_assert(sizeof(&(va_arg)) > 0, "va_arg");
#endif
#ifndef va_copy
static_assert(sizeof(&(va_copy)) > 0, "va_copy");
#endif
#ifndef va_end
static_assert(sizeof(&(va_end)) > 0, "va_end");
#endif
#ifndef va_start
static_assert(sizeof(&(va_start)) > 0, "va_start");
#endif
#ifndef atomic_init
static_assert(sizeof(&(atomic_init)) > 0, "atomic_init");
#endif
#ifndef kill_dependency
static_assert(sizeof(&(kill_dependency)) > 0, "kill_dependency");
#endif
#ifndef atomic_is_lock_free
static_assert(sizeof(&(atomic_is_lock_free)) > 0, "atomic_is_lock_free");
#endif
#ifndef atomic_store
static_assert(sizeof(&(atomic_store)) > 0, "atomic_store");
#endif
#ifndef atomic_store_explicit
static_assert(sizeof(&(atomic_store_explicit)) > 0, "atomic_store_explicit");
#endif
#ifndef atomic_load
static_assert(sizeof(&(atomic_load)) > 0, "atomic_load");
#endif
#ifndef atomic_load_explicit
static_assert(sizeof(&(atomic_load_explicit)) > 0, "atomic_load_explicit");
#endif
#ifndef atomic_exchange
static_assert(sizeof(&(atomic_exchange)) > 0, "atomic_exchange");
#endif
#ifndef atomic_exchange_explicit
static_assert(sizeof(&(atomic_exchange_explicit)) > 0, "atomic_exchange_explicit");
#endif
#ifndef atomic_compare_exchange_strong
static_assert(sizeof(&(atomic_compare_exchange_strong)) > 0, "atomic_compare_exchange_strong");
#endif
#ifndef atomic_compare_exchange_strong_explicit
static_assert(sizeof(&(atomic_compare_exchange_strong_explicit)) > 0, "atomic_compare_exchange_strong_explicit");
#endif
#ifndef atomic_compare_exchange_weak
static_assert(sizeof(&(atomic_compare_exchange_weak)) > 0, "atomic_compare_exchange_weak");
#endif
#ifndef atomic_compare_exchange_weak_explicit
static_assert(sizeof(&(atomic_compare_exchange_weak_explicit)) > 0, "atomic_compare_exchange_weak_explicit");
#endif
#ifndef atomic_fetch_add
static_assert(sizeof(&(atomic_fetch_add)) > 0, "atomic_fetch_add");
#endif
#ifndef atomic_fetch_sub
static_assert(sizeof(&(atomic_fetch_sub)) > 0, "atomic_fetch_sub");
#endif
#ifndef atomic_fetch_or
static_assert(sizeof(&(atomic_fetch_or)) > 0, "atomic_fetch_or");
#endif
#ifndef atomic_fetch_xor
static_assert(sizeof(&(atomic_fetch_xor)) > 0, "atomic_fetch_xor");
#endif
#ifndef atomic_fetch_and
static_assert(sizeof(&(atomic_fetch_and)) > 0, "atomic_fetch_and");
#endif
#ifndef atomic_fetch_add_explicit
static_assert(sizeof(&(atomic_fetch_add_explicit)) > 0, "atomic_fetch_add_explicit");
#endif
#ifndef atomic_fetch_sub_explicit
static_assert(sizeof(&(atomic_fetch_sub_explicit)) > 0, "atomic_fetch_sub_explicit");
#endif
#ifndef atomic_fetch_or_explicit
static_assert(sizeof(&(atomic_fetch_or_explicit)) > 0, "atomic_fetch_or_explicit");
#endif
#ifndef atomic_fetch_xor_explicit
static_assert(sizeof(&(atomic_fetch_xor_explicit)) > 0, "atomic_fetch_xor_explicit");
#endif
#ifndef atomic_fetch_and_explicit
static_assert(sizeof(&(atomic_fetch_and_explicit)) > 0, "atomic_fetch_and_explicit");
#endif
#ifndef stdc_leading_zeros
static_assert(sizeof(&(stdc_leading_zeros)) > 0, "stdc_leading_zeros");
#endif
#ifndef stdc_leading_ones
static_assert(sizeof(&(stdc_leading_ones)) > 0, "stdc_leading_ones");
#endif
#ifndef stdc_trailing_zeros
static_assert(sizeof(&(stdc_trailing_zeros)) > 0, "stdc_trailing_zeros");
#endif
#ifndef stdc_trailing_ones
static_assert(sizeof(&(stdc_trailing_ones)) > 0, "stdc_trailing_ones");
#endif
#ifndef stdc_first_leading_zero
static_assert(sizeof(&(stdc_first_leading_zero)) > 0, "stdc_first_leading_zero");
#endif
#ifndef stdc_first_leading_one
static_assert(sizeof(&(stdc_first_leading_one)) > 0, "stdc_first_leading_one");
#endif
#ifndef stdc_first_trailing_zero
static_assert(sizeof(&(stdc_first_trailing_zero)) > 0, "stdc_first_trailing_zero");
#endif
#ifndef stdc_first_trailing_one
static_assert(sizeof(&(stdc_first_trailing_one)) > 0, "stdc_first_trailing_one");
#endif
#ifndef stdc_count_zeros
static_assert(sizeof(&(stdc_count_zeros)) > 0, "stdc_count_zeros");
#endif
#ifndef stdc_count_ones
static_assert(sizeof(&(stdc_count_ones)) > 0, "stdc_count_ones");
#endif
#ifndef stdc_has_single_bit
static_assert(sizeof(&(stdc_has_single_bit)) > 0, "stdc_has_single_bit");
#endif
#ifndef stdc_bit_width
static_assert(sizeof(&(stdc_bit_width)) > 0, "stdc_bit_width");
#endif
#ifndef stdc_bit_floor
static_assert(sizeof(&(stdc_bit_floor)) > 0, "stdc_bit_floor");
#endif
#ifndef stdc_bit_ceil
static_assert(sizeof(&(stdc_bit_ceil)) > 0, "stdc_bit_ceil");
#endif
#ifndef ckd_add
static_assert(sizeof(&(ckd_add)) > 0, "ckd_add");
#endif
#ifndef ckd_sub
static_assert(sizeof(&(ckd_sub)) > 0, "ckd_sub");
#endif
#ifndef ckd_mul
static_assert(sizeof(&(ckd_mul)) > 0, "ckd_mul");
#endif
#ifndef bsearch
static_assert(sizeof(&(bsearch)) > 0, "bsearch");
#endif
#ifndef memchr
static_assert(sizeof(&(memchr)) > 0, "memchr");
#endif
#ifndef strchr
static_assert(sizeof(&(strchr)) > 0, "strchr");
#endif
#ifndef strpbrk
static_assert(sizeof(&(strpbrk)) > 0, "strpbrk");
#endif
#ifndef strrchr
static_assert(sizeof(&(strrchr)) > 0, "strrchr");
#endif
#ifndef strstr
static_assert(sizeof(&(strstr)) > 0, "strstr");
#endif
#ifndef mtx_init
static_assert(sizeof(&(mtx_init)) > 0, "mtx_init");
#endif

int main(void)
{
	return 0;
}
