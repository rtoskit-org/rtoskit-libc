/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * posix.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* A POSIX feature test macro shows the subset's names in the ISO headers (PLAN D128, D132) */
#define _POSIX_C_SOURCE 202405L

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "api.h"

#define SAME_TYPE(expr, type) _Generic((expr), type: true, default: false)

static_assert(SAME_TYPE(&fileno, int (*)(FILE *)), "fileno");
static_assert(SAME_TYPE(&fdopen, FILE *(*)(int, const char *)), "fdopen");
static_assert(SAME_TYPE(&strnlen, size_t (*)(const char *, size_t)), "strnlen");
static_assert(SAME_TYPE(&reallocarray, void *(*)(void *, size_t, size_t)), "reallocarray");
static_assert(SAME_TYPE(&sincos, void (*)(double, double *, double *)), "sincos");
static_assert(SAME_TYPE(&sincosf, void (*)(float, float *, float *)), "sincosf");
static_assert(SAME_TYPE(&clock_gettime, int (*)(clockid_t, struct timespec *)), "clock_gettime");
static_assert(SAME_TYPE(&clock_getres, int (*)(clockid_t, struct timespec *)), "clock_getres");
static_assert(SAME_TYPE(&nanosleep, int (*)(const struct timespec *, struct timespec *)), "nanosleep");
static_assert(SAME_TYPE((clockid_t)0, int), "clockid_t is <sys/types.h>'s");

/* A clock is a seam time base */
static_assert(CLOCK_REALTIME == TIME_UTC && CLOCK_MONOTONIC == TIME_MONOTONIC && CLOCK_PROCESS_CPUTIME_ID == TIME_ACTIVE && CLOCK_THREAD_CPUTIME_ID == TIME_THREAD_ACTIVE, "<time.h> clocks");

bool posix_visible(void)
{
	return CLOCK_MONOTONIC == 2;
}
