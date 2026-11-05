/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_cond_timedwait.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

#include "internal/pthread.h"

/* POSIX pthread_cond_timedwait: abstime on the condition variable's clock (pthread_condattr_setclock) */
int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	return __libc_pthread_cond_wait(cond, mutex, __libc_pthread_clock_base(cond->__clock), abstime);
}
