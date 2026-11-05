/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * features.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <rtoskit-libc.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "internal/cxa.h"

/*
 * <rtoskit-libc.h> against the library (issue #125, PLAN D147): every
 * feature the header names is checked here, at compile time by its macro
 * and at link time by its function, and features.run fails when the header
 * names a feature this file does not check. Including the headers above is
 * the check for the __LIBC_HAVE_*_H features.
 */
#define FUNCTIONS(X)                            \
	X(ACOSF, acosf)                             \
	X(ASINF, asinf)                             \
	X(ATAN2F, atan2f)                           \
	X(ATANF, atanf)                             \
	X(CEILF, ceilf)                             \
	X(COSF, cosf)                               \
	X(COSHF, coshf)                             \
	X(EXPF, expf)                               \
	X(FABSF, fabsf)                             \
	X(FLOORF, floorf)                           \
	X(FMODF, fmodf)                             \
	X(FREXPF, frexpf)                           \
	X(HYPOTF, hypotf)                           \
	X(LDEXPF, ldexpf)                           \
	X(LOG10F, log10f)                           \
	X(LOGF, logf)                               \
	X(MODFF, modff)                             \
	X(POWF, powf)                               \
	X(SINF, sinf)                               \
	X(SINHF, sinhf)                             \
	X(SQRTF, sqrtf)                             \
	X(TANF, tanf)                               \
	X(TANHF, tanhf)                             \
	X(HYPOT, hypot)                             \
	X(ACOSL, acosl)                             \
	X(ASINL, asinl)                             \
	X(ATAN2L, atan2l)                           \
	X(ATANL, atanl)                             \
	X(CEILL, ceill)                             \
	X(COSHL, coshl)                             \
	X(COSL, cosl)                               \
	X(EXPL, expl)                               \
	X(FABSL, fabsl)                             \
	X(FLOORL, floorl)                           \
	X(FMODL, fmodl)                             \
	X(FREXPL, frexpl)                           \
	X(HYPOTL, hypotl)                           \
	X(LDEXPL, ldexpl)                           \
	X(LOG10L, log10l)                           \
	X(LOGL, logl)                               \
	X(MODFL, modfl)                             \
	X(POWL, powl)                               \
	X(SINHL, sinhl)                             \
	X(SINL, sinl)                               \
	X(SQRTL, sqrtl)                             \
	X(TANHL, tanhl)                             \
	X(TANL, tanl)                               \
	X(STRTOF, strtof)                           \
	X(ALIGNED_ALLOC, aligned_alloc)             \
	X(AT_QUICK_EXIT, at_quick_exit)             \
	X(QUICK_EXIT, quick_exit)                   \
	X(__CXA_THREAD_ATEXIT, __cxa_thread_atexit) \
	X(GETENTROPY, getentropy)                   \
	X(NANOSLEEP, nanosleep)                     \
	X(SCHED_YIELD, sched_yield)

#define MACRO_IS_ONE(name, function) static_assert(__LIBC_HAVE_##name == 1, "__LIBC_HAVE_" #name);
#define ADDRESS(name, function) (void (*)(void)) function,

FUNCTIONS(MACRO_IS_ONE)

/* The features that are not functions */
static_assert(__LIBC_HAVE_TLS == 1, "__LIBC_HAVE_TLS");
static_assert(__LIBC_HAVE_SYS_TYPES_H == 1 && __LIBC_HAVE_SYS_STAT_H == 1 && __LIBC_HAVE_INTTYPES_H == 1, "__LIBC_HAVE_*_H");
static_assert(__LIBC_HAVE_FCNTL_H == 1 && __LIBC_HAVE_UNISTD_H == 1, "__LIBC_HAVE_*_H");
static_assert(__LIBC_HAVE_S_ISREG == 1 && S_ISREG(S_IFREG) && !S_ISREG(S_IFDIR), "__LIBC_HAVE_S_ISREG");
static_assert(__LIBC_HAVE_CLOCK_REALTIME == 1 && __LIBC_HAVE_CLOCK_MONOTONIC == 1 && CLOCK_REALTIME != CLOCK_MONOTONIC, "__LIBC_HAVE_CLOCK_*");
static_assert(__RTOSKIT_LIBC__ == 1, "__RTOSKIT_LIBC__");

/* Every function, so each must link */
static void (*const volatile functions[])(void) = { FUNCTIONS(ADDRESS) };

static thread_local int tls_value = 7;

static int failures;

static void check(bool ok, const char *what)
{
	if (!ok) {
		printf("fail: %s\n", what);
		++failures;
	}
}

int main(void)
{
	struct timespec now;
	unsigned char bytes[16];
	void *block = aligned_alloc(64, 128);

	for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i)
		check(functions[i] != nullptr, "every function links");

	/* The ones libstdc++ calls at run time, on the Linux test port */
	check(tls_value == 7, "thread_local");
	check(strtof("1.5", nullptr) == 1.5f, "strtof");
	check(block != nullptr && ((uintptr_t)block & 63) == 0, "aligned_alloc");
	free(block);
	check(clock_gettime(CLOCK_REALTIME, &now) == 0 && clock_gettime(CLOCK_MONOTONIC, &now) == 0, "clock_gettime");
	check(nanosleep(&(struct timespec){ .tv_nsec = 1000 }, nullptr) == 0, "nanosleep");
	/* libstdc++'s _GLIBCXX_USE_SCHED_YIELD reaches thrd_yield through gthr-c11 (issue #125) */
	check(sched_yield() == 0, "sched_yield");
	thrd_yield();
	check(getentropy(bytes, sizeof(bytes)) == 0, "getentropy");

	if (failures == 0)
		puts("pass: features");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
