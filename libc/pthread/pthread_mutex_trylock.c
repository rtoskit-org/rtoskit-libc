/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutex_trylock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/threads.h"

/* POSIX pthread_mutex_trylock: EBUSY when another holds it, or the caller holds a non-recursive one */
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	pthread_t self = __rtos_thread_self();
	unsigned int state = __LIBC_MUTEX_UNLOCKED;

	if (mutex->__type == PTHREAD_MUTEX_RECURSIVE && __atomic_load_n(&mutex->__owner, __ATOMIC_RELAXED) == self) {
		if (mutex->__count == UINT_MAX)
			return EAGAIN;
		++mutex->__count;
		return 0;
	}

	if (!__atomic_compare_exchange_n(&mutex->__state, &state, __LIBC_MUTEX_LOCKED, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		return EBUSY;

	__atomic_store_n(&mutex->__owner, self, __ATOMIC_RELAXED);
	mutex->__count = 1;

	return 0;
}
