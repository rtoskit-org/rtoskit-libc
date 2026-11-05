/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutex_timedlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <time.h>

#include "internal/pthread.h"

/* POSIX pthread_mutex_timedlock: abstime on CLOCK_REALTIME; ETIMEDOUT once it passes */
int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	return __libc_pthread_mutex_lock(mutex, TIME_UTC, abstime);
}
