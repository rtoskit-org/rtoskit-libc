/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_cond_wait.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"

/*
 * Read the sequence while holding the mutex, release it, and sleep while the
 * sequence is unchanged; a signal in between changes it, so no wake is lost
 * (docs/research/threads.md). A recursive mutex is released fully and its
 * count restored; the mutex is taken back with no deadline. EPERM when the
 * caller does not hold the mutex.
 */
int __libc_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, int base, const struct timespec *deadline)
{
	unsigned int sequence = __atomic_load_n(&cond->__sequence, __ATOMIC_RELAXED);
	unsigned int count = mutex->__count;
	int status;

	if (__atomic_load_n(&mutex->__owner, __ATOMIC_RELAXED) != __rtos_thread_self() || count == 0)
		return EPERM;

	mutex->__count = 1;
	__libc_pthread_mutex_unlock(mutex);

	status = __rtos_futex_wait(&cond->__sequence, sequence, base, deadline);

	while (__libc_pthread_mutex_lock(mutex, base, nullptr) != 0)
		continue;
	mutex->__count = count;

	return status == -ETIMEDOUT || status == -EINVAL ? -status : 0;
}
