/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_rwlock_wrlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/* A writer takes a lock no one holds (P10.06, PLAN D155); EDEADLK when the caller is already the writer */
int __libc_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock, int base, const struct timespec *deadline)
{
	pthread_t self = __rtos_thread_self();

	if (__atomic_load_n(&rwlock->__writer, __ATOMIC_RELAXED) == self)
		return EDEADLK;

	while (true) {
		unsigned int sequence = __atomic_load_n(&rwlock->__sequence, __ATOMIC_ACQUIRE);
		unsigned int state = 0;

		if (__atomic_compare_exchange_n(&rwlock->__state, &state, __LIBC_RWLOCK_WRITER, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
			__atomic_store_n(&rwlock->__writer, self, __ATOMIC_RELAXED);
			return 0;
		}

		int status = __rtos_futex_wait(&rwlock->__sequence, sequence, base, deadline);

		if (status == -ETIMEDOUT || status == -EINVAL)
			return -status;
	}
}
