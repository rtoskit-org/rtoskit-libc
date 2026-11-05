/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_barrier_destroy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_barrier_destroy: EBUSY while threads wait at it */
int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	return __atomic_load_n(&barrier->__arrived, __ATOMIC_ACQUIRE) != 0 ? EBUSY : 0;
}
