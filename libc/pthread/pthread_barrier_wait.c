/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_barrier_wait.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * POSIX pthread_barrier_wait (P10.06, PLAN D155): the last of count threads
 * to arrive opens the barrier, by resetting the arrivals and advancing the
 * generation the others sleep on, and gets PTHREAD_BARRIER_SERIAL_THREAD;
 * the others get 0
 */
int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	unsigned int generation = __atomic_load_n(&barrier->__generation, __ATOMIC_ACQUIRE);

	if (__atomic_add_fetch(&barrier->__arrived, 1, __ATOMIC_ACQ_REL) == barrier->__count) {
		__atomic_store_n(&barrier->__arrived, 0, __ATOMIC_RELAXED);
		__atomic_fetch_add(&barrier->__generation, 1, __ATOMIC_RELEASE);
		__rtos_futex_wake(&barrier->__generation, UINT_MAX);
		return PTHREAD_BARRIER_SERIAL_THREAD;
	}

	while (__atomic_load_n(&barrier->__generation, __ATOMIC_ACQUIRE) == generation)
		__rtos_futex_wait(&barrier->__generation, generation, TIME_UTC, nullptr);

	return 0;
}
