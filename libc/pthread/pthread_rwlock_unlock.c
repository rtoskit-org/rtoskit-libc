/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_unlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/*
 * POSIX pthread_rwlock_unlock: the writer, or one reader, lets go; a release
 * that frees the lock advances the sequence and wakes every waiter (P10.06).
 * EPERM when no one holds it, or another thread is the writer.
 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	unsigned int state = __atomic_load_n(&rwlock->__state, __ATOMIC_RELAXED);

	if (state == __LIBC_RWLOCK_WRITER) {
		if (__atomic_load_n(&rwlock->__writer, __ATOMIC_RELAXED) != __rtos_thread_self())
			return EPERM;
		__atomic_store_n(&rwlock->__writer, nullptr, __ATOMIC_RELAXED);
		__atomic_store_n(&rwlock->__state, 0, __ATOMIC_RELEASE);
	} else {
		do {
			if (state == 0)
				return EPERM;
		} while (!__atomic_compare_exchange_n(&rwlock->__state, &state, state - 1, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
		if (state != 1)
			return 0;
	}

	__atomic_fetch_add(&rwlock->__sequence, 1, __ATOMIC_RELEASE);
	__rtos_futex_wake(&rwlock->__sequence, UINT_MAX);

	return 0;
}
