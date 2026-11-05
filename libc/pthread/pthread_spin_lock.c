/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_spin_lock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <rtos/rtos-port.h>

/*
 * POSIX pthread_spin_lock: spin until the lock is free, yielding to the
 * scheduler between tries so that a lower-priority holder on the same core
 * can run and let go (P10.06, PLAN D155)
 */
int pthread_spin_lock(pthread_spinlock_t *lock)
{
	while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE) != 0)
		__rtos_thread_yield();

	return 0;
}
