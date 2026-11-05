/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_tryrdlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/* POSIX pthread_rwlock_tryrdlock: EBUSY while a writer holds it */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	unsigned int state = __atomic_load_n(&rwlock->__state, __ATOMIC_RELAXED);

	while (state != __LIBC_RWLOCK_WRITER) {
		if (state == __LIBC_RWLOCK_WRITER - 1)
			return EAGAIN;
		if (__atomic_compare_exchange_n(&rwlock->__state, &state, state + 1, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
			return 0;
	}

	return EBUSY;
}
