/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_trywrlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/* POSIX pthread_rwlock_trywrlock: EBUSY while anyone holds it */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	unsigned int state = 0;

	if (!__atomic_compare_exchange_n(&rwlock->__state, &state, __LIBC_RWLOCK_WRITER, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		return EBUSY;
	__atomic_store_n(&rwlock->__writer, __rtos_thread_self(), __ATOMIC_RELAXED);

	return 0;
}
