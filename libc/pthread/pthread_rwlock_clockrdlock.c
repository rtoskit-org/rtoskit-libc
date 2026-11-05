/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_clockrdlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "internal/pthread.h"

/* POSIX pthread_rwlock_clockrdlock: abstime on CLOCK_REALTIME or CLOCK_MONOTONIC (PLAN D150); EINVAL for another clock */
int pthread_rwlock_clockrdlock(pthread_rwlock_t *restrict rwlock, clockid_t clock_id, const struct timespec *restrict abstime)
{
	int base = __libc_pthread_clock_base(clock_id);

	if (base < 0)
		return EINVAL;

	return __libc_pthread_rwlock_rdlock(rwlock, base, abstime);
}
