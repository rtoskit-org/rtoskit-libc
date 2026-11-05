/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_rwlock_rdlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/*
 * A reader joins unless a writer holds the lock; readers go first, so a
 * thread that already reads can always read again (P10.06, PLAN D155).
 * EDEADLK when the caller is the writer, EAGAIN at the most readers.
 */
int __libc_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock, int base, const struct timespec *deadline)
{
	if (__atomic_load_n(&rwlock->__writer, __ATOMIC_RELAXED) == __rtos_thread_self())
		return EDEADLK;

	while (true) {
		unsigned int sequence = __atomic_load_n(&rwlock->__sequence, __ATOMIC_ACQUIRE);
		unsigned int state = __atomic_load_n(&rwlock->__state, __ATOMIC_RELAXED);

		if (state == __LIBC_RWLOCK_WRITER - 1)
			return EAGAIN;
		if (state != __LIBC_RWLOCK_WRITER) {
			if (__atomic_compare_exchange_n(&rwlock->__state, &state, state + 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
				return 0;
			continue;
		}

		int status = __rtos_futex_wait(&rwlock->__sequence, sequence, base, deadline);

		if (status == -ETIMEDOUT || status == -EINVAL)
			return -status;
	}
}
