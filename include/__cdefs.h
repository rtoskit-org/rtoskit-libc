/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __cdefs.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef ___CDEFS_H
#define ___CDEFS_H

/*
 * What the public headers need to compile as C++ as well as C (PLAN D120,
 * issue #98): C linkage for every declaration, and a null pointer constant
 * C++ accepts. The headers spell restrict, typeof, thread_local, alignas and
 * the attributes the GNU way (__restrict, __typeof__, __thread,
 * __attribute__), which GCC and Clang take in both languages and in every
 * C++ mode, C++98 included (D131), so these are the only conditionals the
 * language needs. __null is the compilers' null pointer constant for C++,
 * which C++98 accepts as well as later modes.
 */
#ifdef __cplusplus
#define __LIBC_BEGIN_DECLS extern "C" {
#define __LIBC_END_DECLS }
#define __LIBC_NULL __null
#else
#define __LIBC_BEGIN_DECLS
#define __LIBC_END_DECLS
#define __LIBC_NULL ((void *)0)
#endif

/*
 * The POSIX subset's names in the ISO C headers (PLAN D128, D132): fileno and
 * fdopen in <stdio.h>, strnlen in <string.h>, reallocarray in <stdlib.h>,
 * sincos and sincosf in <math.h>, and clock_gettime, clock_getres, nanosleep
 * and the CLOCK_* clocks in <time.h>. A strictly conforming program may use
 * these names itself (C23 7.1.3), so they are hidden in the strict modes
 * (-std=c23, which defines __STRICT_ANSI__) unless the program asks for them
 * with a POSIX or GNU feature test macro, as the POSIX headers expect. The GNU
 * modes and C++ (g++ defines _GNU_SOURCE) see them.
 */
#if !defined(__STRICT_ANSI__) || defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_DEFAULT_SOURCE)
#define __LIBC_POSIX_VISIBLE 1
#else
#define __LIBC_POSIX_VISIBLE 0
#endif

#endif /* ___CDEFS_H */
