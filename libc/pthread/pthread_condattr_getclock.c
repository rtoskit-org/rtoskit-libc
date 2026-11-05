/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_condattr_getclock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <pthread.h>
#include <time.h>

/* POSIX pthread_condattr_getclock */
int pthread_condattr_getclock(const pthread_condattr_t *restrict attr, clockid_t *restrict clock_id)
{
	*clock_id = attr->__clock;

	return 0;
}
