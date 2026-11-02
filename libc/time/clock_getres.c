/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * clock_getres.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <rtos/rtos-port.h>
#include <time.h>

/* POSIX.1-2024 clock_getres: a clock is a seam time base (CLOCK_REALTIME is TIME_UTC, PLAN D132) */
int clock_getres(clockid_t clock_id, struct timespec *res)
{
	int status = __rtos_clock_getres(clock_id, res);

	if (status < 0) {
		errno = -status;
		return -1;
	}

	return 0;
}
