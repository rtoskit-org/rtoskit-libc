/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * nanosleep.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * POSIX.1-2024 nanosleep, over the seam's thread sleep: EINVAL for a
 * tv_nsec outside [0, 1e9) and EINTR, with the time left in rmtp, for an
 * interrupted sleep
 */
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
	int status = __rtos_thread_sleep(rqtp, rmtp);

	if (status < 0) {
		errno = -status;
		return -1;
	}

	return 0;
}
