/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlock_destroy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_rwlock_destroy: EBUSY while anyone holds it */
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	return __atomic_load_n(&rwlock->__state, __ATOMIC_RELAXED) != 0 ? EBUSY : 0;
}
