/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_mutex_unlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

#include "internal/pthread.h"
#include "internal/threads.h"

/* Only the owner unlocks (EPERM otherwise); the last unlock wakes one waiter if any may sleep */
int __libc_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (__atomic_load_n(&mutex->__owner, __ATOMIC_RELAXED) != __rtos_thread_self() || mutex->__count == 0)
		return EPERM;
	if (--mutex->__count != 0)
		return 0;

	__atomic_store_n(&mutex->__owner, nullptr, __ATOMIC_RELAXED);
	if (mutex->__protocol == PTHREAD_PRIO_INHERIT)
		return -__rtos_mutex_unlock_pi(&mutex->__state);
	if (__atomic_exchange_n(&mutex->__state, __LIBC_MUTEX_UNLOCKED, __ATOMIC_RELEASE) == __LIBC_MUTEX_CONTENDED)
		__rtos_futex_wake(&mutex->__state, 1);

	return 0;
}
