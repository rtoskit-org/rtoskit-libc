/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * rtoskit-libc.h
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _RTOSKIT_LIBC_H
#define _RTOSKIT_LIBC_H

/*
 * What the library provides, for a toolchain's configure (issue #125, PLAN
 * D147), as <picolibc.h> serves libstdc++'s --with-picolibc branch.
 * libstdc++'s configure runs no link tests on a cross target, so instead of
 * a hard-coded list it compiles a probe per feature against this header:
 *
 *     #include <rtoskit-libc.h>
 *     #ifndef __LIBC_HAVE_SINF
 *     #error
 *     #endif
 *
 * Each feature is a name in autoconf's spelling (a function or macro in
 * upper case, a header as FOO_H, or SYS_FOO_H), so __LIBC_HAVE_SINF answers
 * libstdc++'s HAVE_SINF, and __LIBC_HAVE_NANOSLEEP its _GLIBCXX_USE_NANOSLEEP.
 * A feature defined here is in libc.a, and its header declares it, for every
 * architecture and every flavour archive; test/unit/features compiles and
 * links each one, and fails when this list and the test disagree.
 *
 * The clocks, sleeping and the entropy source exist in the library, but
 * reach the OS through the seam: a port without a clock or an entropy
 * source fails them at run time (-ENOSYS), which a header cannot say.
 */
#define __RTOSKIT_LIBC__ 1

/* <math.h>: the float functions libstdc++'s <cmath> uses */
#define __LIBC_HAVE_ACOSF 1
#define __LIBC_HAVE_ASINF 1
#define __LIBC_HAVE_ATAN2F 1
#define __LIBC_HAVE_ATANF 1
#define __LIBC_HAVE_CEILF 1
#define __LIBC_HAVE_COSF 1
#define __LIBC_HAVE_COSHF 1
#define __LIBC_HAVE_EXPF 1
#define __LIBC_HAVE_FABSF 1
#define __LIBC_HAVE_FLOORF 1
#define __LIBC_HAVE_FMODF 1
#define __LIBC_HAVE_FREXPF 1
#define __LIBC_HAVE_HYPOTF 1
#define __LIBC_HAVE_LDEXPF 1
#define __LIBC_HAVE_LOG10F 1
#define __LIBC_HAVE_LOGF 1
#define __LIBC_HAVE_MODFF 1
#define __LIBC_HAVE_POWF 1
#define __LIBC_HAVE_SINF 1
#define __LIBC_HAVE_SINHF 1
#define __LIBC_HAVE_SQRTF 1
#define __LIBC_HAVE_TANF 1
#define __LIBC_HAVE_TANHF 1

/* <math.h>: the double one libstdc++ checks, and the long double forms */
#define __LIBC_HAVE_HYPOT 1
#define __LIBC_HAVE_ACOSL 1
#define __LIBC_HAVE_ASINL 1
#define __LIBC_HAVE_ATAN2L 1
#define __LIBC_HAVE_ATANL 1
#define __LIBC_HAVE_CEILL 1
#define __LIBC_HAVE_COSHL 1
#define __LIBC_HAVE_COSL 1
#define __LIBC_HAVE_EXPL 1
#define __LIBC_HAVE_FABSL 1
#define __LIBC_HAVE_FLOORL 1
#define __LIBC_HAVE_FMODL 1
#define __LIBC_HAVE_FREXPL 1
#define __LIBC_HAVE_HYPOTL 1
#define __LIBC_HAVE_LDEXPL 1
#define __LIBC_HAVE_LOG10L 1
#define __LIBC_HAVE_LOGL 1
#define __LIBC_HAVE_MODFL 1
#define __LIBC_HAVE_POWL 1
#define __LIBC_HAVE_SINHL 1
#define __LIBC_HAVE_SINL 1
#define __LIBC_HAVE_SQRTL 1
#define __LIBC_HAVE_TANHL 1
#define __LIBC_HAVE_TANL 1

/* <stdlib.h> */
#define __LIBC_HAVE_STRTOF 1
#define __LIBC_HAVE_ALIGNED_ALLOC 1
#define __LIBC_HAVE_AT_QUICK_EXIT 1
#define __LIBC_HAVE_QUICK_EXIT 1

/* Thread-local storage, and the C++ ABI's thread_local destructors (PLAN D121) */
#define __LIBC_HAVE_TLS 1
#define __LIBC_HAVE___CXA_THREAD_ATEXIT 1

/* The POSIX subset (PLAN D128): its headers, S_ISREG and getentropy (D146) */
#define __LIBC_HAVE_SYS_TYPES_H 1
#define __LIBC_HAVE_SYS_STAT_H 1
#define __LIBC_HAVE_INTTYPES_H 1
#define __LIBC_HAVE_FCNTL_H 1
#define __LIBC_HAVE_UNISTD_H 1
#define __LIBC_HAVE_S_ISREG 1
#define __LIBC_HAVE_GETENTROPY 1

/* Clocks, sleeping and yielding over the seam: libstdc++'s _GLIBCXX_USE_* names */
#define __LIBC_HAVE_CLOCK_REALTIME 1
#define __LIBC_HAVE_CLOCK_MONOTONIC 1
#define __LIBC_HAVE_NANOSLEEP 1
#define __LIBC_HAVE_SCHED_YIELD 1

#endif /* _RTOSKIT_LIBC_H */
