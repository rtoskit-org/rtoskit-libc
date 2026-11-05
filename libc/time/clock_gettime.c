/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * clock_gettime.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * POSIX.1-2024 clock_gettime: a clock is a seam time base (CLOCK_REALTIME is
 * TIME_UTC, PLAN D132). *tp is unspecified on failure; it is zeroed, after
 * the seam call so a port's partial write is covered too, because callers
 * such as libstdc++'s clocks ignore the status (issue #122).
 */
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	int status = __rtos_clock_gettime(clock_id, tp);

	if (status < 0) {
		tp->tv_sec = 0;
		tp->tv_nsec = 0;
		errno = -status;
		return -1;
	}

	return 0;
}
