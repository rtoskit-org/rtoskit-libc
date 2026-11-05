/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_condattr_setclock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <pthread.h>
#include <time.h>

/* POSIX pthread_condattr_setclock: CLOCK_REALTIME or CLOCK_MONOTONIC; the CPU-time clocks are EINVAL */
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
	if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC)
		return EINVAL;
	attr->__clock = clock_id;

	return 0;
}
