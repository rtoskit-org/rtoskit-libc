/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * timespec_get.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * C23 7.29.2.6: base on success, 0 on failure. *ts is unspecified on
 * failure; it is zeroed so a caller that ignores the result reads no stale
 * time (issue #122).
 */
int timespec_get(struct timespec *ts, int base)
{
	int status = __rtos_clock_gettime(base, ts);

	if (status < 0) {
		ts->tv_sec = 0;
		ts->tv_nsec = 0;
		errno = -status;
		return 0;
	}

	return base;
}
