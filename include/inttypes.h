/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * inttypes.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTTYPES_H
#define _INTTYPES_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

#include <stdint.h>

/* C23 7.8 */
#define __STDC_VERSION_INTTYPES_H__ 202311L

typedef struct {
	intmax_t quot;
	intmax_t rem;
} imaxdiv_t;

/*
 * C23 7.8.1: the conversion macros use the width length modifiers of
 * 7.23.6.1p7 (wN for the exact- and minimum-width types, wfN for the fastest
 * ones), so one header serves every target with no per-target length letters
 * (PLAN D15, D33). intptr_t and ptrdiff_t are the same type on every
 * supported target, so the PTR macros use t.
 */
#ifndef __cplusplus
static_assert(_Generic((intptr_t)0, typeof((__PTRDIFF_TYPE__)0): 1, default: 0), "PRIdPTR uses the ptrdiff_t length modifier");
#endif

#define PRId8 "w8d"
#define PRId16 "w16d"
#define PRId32 "w32d"
#define PRId64 "w64d"
#define PRIdLEAST8 "w8d"
#define PRIdLEAST16 "w16d"
#define PRIdLEAST32 "w32d"
#define PRIdLEAST64 "w64d"
#define PRIdFAST8 "wf8d"
#define PRIdFAST16 "wf16d"
#define PRIdFAST32 "wf32d"
#define PRIdFAST64 "wf64d"
#define PRIdMAX "jd"
#define PRIdPTR "td"

#define PRIi8 "w8i"
#define PRIi16 "w16i"
#define PRIi32 "w32i"
#define PRIi64 "w64i"
#define PRIiLEAST8 "w8i"
#define PRIiLEAST16 "w16i"
#define PRIiLEAST32 "w32i"
#define PRIiLEAST64 "w64i"
#define PRIiFAST8 "wf8i"
#define PRIiFAST16 "wf16i"
#define PRIiFAST32 "wf32i"
#define PRIiFAST64 "wf64i"
#define PRIiMAX "ji"
#define PRIiPTR "ti"

#define PRIo8 "w8o"
#define PRIo16 "w16o"
#define PRIo32 "w32o"
#define PRIo64 "w64o"
#define PRIoLEAST8 "w8o"
#define PRIoLEAST16 "w16o"
#define PRIoLEAST32 "w32o"
#define PRIoLEAST64 "w64o"
#define PRIoFAST8 "wf8o"
#define PRIoFAST16 "wf16o"
#define PRIoFAST32 "wf32o"
#define PRIoFAST64 "wf64o"
#define PRIoMAX "jo"
#define PRIoPTR "to"

#define PRIu8 "w8u"
#define PRIu16 "w16u"
#define PRIu32 "w32u"
#define PRIu64 "w64u"
#define PRIuLEAST8 "w8u"
#define PRIuLEAST16 "w16u"
#define PRIuLEAST32 "w32u"
#define PRIuLEAST64 "w64u"
#define PRIuFAST8 "wf8u"
#define PRIuFAST16 "wf16u"
#define PRIuFAST32 "wf32u"
#define PRIuFAST64 "wf64u"
#define PRIuMAX "ju"
#define PRIuPTR "tu"

#define PRIx8 "w8x"
#define PRIx16 "w16x"
#define PRIx32 "w32x"
#define PRIx64 "w64x"
#define PRIxLEAST8 "w8x"
#define PRIxLEAST16 "w16x"
#define PRIxLEAST32 "w32x"
#define PRIxLEAST64 "w64x"
#define PRIxFAST8 "wf8x"
#define PRIxFAST16 "wf16x"
#define PRIxFAST32 "wf32x"
#define PRIxFAST64 "wf64x"
#define PRIxMAX "jx"
#define PRIxPTR "tx"

#define PRIX8 "w8X"
#define PRIX16 "w16X"
#define PRIX32 "w32X"
#define PRIX64 "w64X"
#define PRIXLEAST8 "w8X"
#define PRIXLEAST16 "w16X"
#define PRIXLEAST32 "w32X"
#define PRIXLEAST64 "w64X"
#define PRIXFAST8 "wf8X"
#define PRIXFAST16 "wf16X"
#define PRIXFAST32 "wf32X"
#define PRIXFAST64 "wf64X"
#define PRIXMAX "jX"
#define PRIXPTR "tX"

#define SCNd8 "w8d"
#define SCNd16 "w16d"
#define SCNd32 "w32d"
#define SCNd64 "w64d"
#define SCNdLEAST8 "w8d"
#define SCNdLEAST16 "w16d"
#define SCNdLEAST32 "w32d"
#define SCNdLEAST64 "w64d"
#define SCNdFAST8 "wf8d"
#define SCNdFAST16 "wf16d"
#define SCNdFAST32 "wf32d"
#define SCNdFAST64 "wf64d"
#define SCNdMAX "jd"
#define SCNdPTR "td"

#define SCNi8 "w8i"
#define SCNi16 "w16i"
#define SCNi32 "w32i"
#define SCNi64 "w64i"
#define SCNiLEAST8 "w8i"
#define SCNiLEAST16 "w16i"
#define SCNiLEAST32 "w32i"
#define SCNiLEAST64 "w64i"
#define SCNiFAST8 "wf8i"
#define SCNiFAST16 "wf16i"
#define SCNiFAST32 "wf32i"
#define SCNiFAST64 "wf64i"
#define SCNiMAX "ji"
#define SCNiPTR "ti"

#define SCNo8 "w8o"
#define SCNo16 "w16o"
#define SCNo32 "w32o"
#define SCNo64 "w64o"
#define SCNoLEAST8 "w8o"
#define SCNoLEAST16 "w16o"
#define SCNoLEAST32 "w32o"
#define SCNoLEAST64 "w64o"
#define SCNoFAST8 "wf8o"
#define SCNoFAST16 "wf16o"
#define SCNoFAST32 "wf32o"
#define SCNoFAST64 "wf64o"
#define SCNoMAX "jo"
#define SCNoPTR "to"

#define SCNu8 "w8u"
#define SCNu16 "w16u"
#define SCNu32 "w32u"
#define SCNu64 "w64u"
#define SCNuLEAST8 "w8u"
#define SCNuLEAST16 "w16u"
#define SCNuLEAST32 "w32u"
#define SCNuLEAST64 "w64u"
#define SCNuFAST8 "wf8u"
#define SCNuFAST16 "wf16u"
#define SCNuFAST32 "wf32u"
#define SCNuFAST64 "wf64u"
#define SCNuMAX "ju"
#define SCNuPTR "tu"

#define SCNx8 "w8x"
#define SCNx16 "w16x"
#define SCNx32 "w32x"
#define SCNx64 "w64x"
#define SCNxLEAST8 "w8x"
#define SCNxLEAST16 "w16x"
#define SCNxLEAST32 "w32x"
#define SCNxLEAST64 "w64x"
#define SCNxFAST8 "wf8x"
#define SCNxFAST16 "wf16x"
#define SCNxFAST32 "wf32x"
#define SCNxFAST64 "wf64x"
#define SCNxMAX "jx"
#define SCNxPTR "tx"

/* C23 7.8.1p6: printf and scanf implement b as specified, so these are defined */
#define PRIb8 "w8b"
#define PRIb16 "w16b"
#define PRIb32 "w32b"
#define PRIb64 "w64b"
#define PRIbLEAST8 "w8b"
#define PRIbLEAST16 "w16b"
#define PRIbLEAST32 "w32b"
#define PRIbLEAST64 "w64b"
#define PRIbFAST8 "wf8b"
#define PRIbFAST16 "wf16b"
#define PRIbFAST32 "wf32b"
#define PRIbFAST64 "wf64b"
#define PRIbMAX "jb"
#define PRIbPTR "tb"

#define PRIB8 "w8B"
#define PRIB16 "w16B"
#define PRIB32 "w32B"
#define PRIB64 "w64B"
#define PRIBLEAST8 "w8B"
#define PRIBLEAST16 "w16B"
#define PRIBLEAST32 "w32B"
#define PRIBLEAST64 "w64B"
#define PRIBFAST8 "wf8B"
#define PRIBFAST16 "wf16B"
#define PRIBFAST32 "wf32B"
#define PRIBFAST64 "wf64B"
#define PRIBMAX "jB"
#define PRIBPTR "tB"

#define SCNb8 "w8b"
#define SCNb16 "w16b"
#define SCNb32 "w32b"
#define SCNb64 "w64b"
#define SCNbLEAST8 "w8b"
#define SCNbLEAST16 "w16b"
#define SCNbLEAST32 "w32b"
#define SCNbLEAST64 "w64b"
#define SCNbFAST8 "wf8b"
#define SCNbFAST16 "wf16b"
#define SCNbFAST32 "wf32b"
#define SCNbFAST64 "wf64b"
#define SCNbMAX "jb"
#define SCNbPTR "tb"

intmax_t imaxabs(intmax_t j);
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
intmax_t strtoimax(const char *__restrict nptr, char **__restrict endptr, int base);
uintmax_t strtoumax(const char *__restrict nptr, char **__restrict endptr, int base);

__LIBC_END_DECLS

#endif /* _INTTYPES_H */
