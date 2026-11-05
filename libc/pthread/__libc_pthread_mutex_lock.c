/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_mutex_lock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"
#include "internal/threads.h"

/*
 * The three-state futex mutex (docs/research/threads.md): 0 -> 1 when free;
 * otherwise mark it contended (2) and sleep until an exchange finds it free.
 * The owner's own relock counts (recursive), fails with EDEADLK
 * (error-checking) or deadlocks as POSIX lets a normal mutex do.
 */
int __libc_pthread_mutex_lock(pthread_mutex_t *mutex, int base, const struct timespec *deadline)
{
	pthread_t self = __rtos_thread_self();
	unsigned int state = __LIBC_MUTEX_UNLOCKED;

	if (__atomic_load_n(&mutex->__owner, __ATOMIC_RELAXED) == self) {
		if (mutex->__type == PTHREAD_MUTEX_RECURSIVE) {
			if (mutex->__count == UINT_MAX)
				return EAGAIN;
			++mutex->__count;
			return 0;
		}
		if (mutex->__type == PTHREAD_MUTEX_ERRORCHECK)
			return EDEADLK;
	}

	if (!__atomic_compare_exchange_n(&mutex->__state, &state, __LIBC_MUTEX_LOCKED, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
		while (__atomic_exchange_n(&mutex->__state, __LIBC_MUTEX_CONTENDED, __ATOMIC_ACQUIRE) != __LIBC_MUTEX_UNLOCKED) {
			int status = __rtos_futex_wait(&mutex->__state, __LIBC_MUTEX_CONTENDED, base, deadline);

			if (status == -ETIMEDOUT || status == -EINVAL)
				return -status;
		}
	}

	__atomic_store_n(&mutex->__owner, self, __ATOMIC_RELAXED);
	mutex->__count = 1;

	return 0;
}
