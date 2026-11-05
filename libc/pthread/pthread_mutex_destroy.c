/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutex_destroy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_mutex_destroy: EBUSY while it is locked; a mutex holds no resources */
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return __atomic_load_n(&mutex->__state, __ATOMIC_RELAXED) != 0 ? EBUSY : 0;
}
