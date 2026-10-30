/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * limits.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LIMITS_H
#define _LIMITS_H

/* C23 7.10 and 5.2.5.3.2, built from the compiler's predefined macros (PLAN D8) */
#define __STDC_VERSION_LIMITS_H__ 202311L

#define BOOL_WIDTH 1
#define BOOL_MAX 1
#define CHAR_BIT __CHAR_BIT__
#define CHAR_WIDTH __CHAR_BIT__
#define SCHAR_WIDTH __SCHAR_WIDTH__
#define UCHAR_WIDTH __SCHAR_WIDTH__
#define SHRT_WIDTH __SHRT_WIDTH__
#define USHRT_WIDTH __SHRT_WIDTH__
#define INT_WIDTH __INT_WIDTH__
#define UINT_WIDTH __INT_WIDTH__
#define LONG_WIDTH __LONG_WIDTH__
#define ULONG_WIDTH __LONG_WIDTH__
#define LLONG_WIDTH __LONG_LONG_WIDTH__
#define ULLONG_WIDTH __LONG_LONG_WIDTH__
/*
 * C23 5.2.4.2.1 requires _BitInt, with BITINT_MAXWIDTH at least ULLONG_WIDTH.
 * A compiler without it (GCC 14 and 15 on 32-bit Arm) predefines no
 * __BITINT_MAXWIDTH__: leave BITINT_MAXWIDTH undefined there rather than
 * expand to an undeclared name (PLAN D127)
 */
#ifdef __BITINT_MAXWIDTH__
#define BITINT_MAXWIDTH __BITINT_MAXWIDTH__
#endif

/* Only the "C" locale is supported, whose multibyte characters are single bytes */
#define MB_LEN_MAX 1

#define SCHAR_MIN (-SCHAR_MAX - 1)
#define SCHAR_MAX __SCHAR_MAX__
#define UCHAR_MAX (SCHAR_MAX * 2 + 1)

/*
 * The signedness of plain char is a compiler option (-fsigned-char) with no
 * predefined value macro, so this is selected here (PLAN D19).
 */
#ifdef __CHAR_UNSIGNED__
#define CHAR_MIN 0
#define CHAR_MAX UCHAR_MAX
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif

#define SHRT_MIN (-SHRT_MAX - 1)
#define SHRT_MAX __SHRT_MAX__
#define USHRT_MAX (SHRT_MAX * 2 + 1)
#define INT_MIN (-INT_MAX - 1)
#define INT_MAX __INT_MAX__
#define UINT_MAX (INT_MAX * 2U + 1U)
#define LONG_MIN (-LONG_MAX - 1L)
#define LONG_MAX __LONG_MAX__
#define ULONG_MAX (LONG_MAX * 2UL + 1UL)
#define LLONG_MIN (-LLONG_MAX - 1LL)
#define LLONG_MAX __LONG_LONG_MAX__
#define ULLONG_MAX (LLONG_MAX * 2ULL + 1ULL)

#endif /* _LIMITS_H */
