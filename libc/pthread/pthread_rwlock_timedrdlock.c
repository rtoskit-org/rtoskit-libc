/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_timedrdlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <time.h>

#include "internal/pthread.h"

/* POSIX pthread_rwlock_timedrdlock: abstime on CLOCK_REALTIME */
int pthread_rwlock_timedrdlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	return __libc_pthread_rwlock_rdlock(rwlock, TIME_UTC, abstime);
}
