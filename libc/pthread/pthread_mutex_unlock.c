/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutex_unlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

#include "internal/pthread.h"

/* POSIX pthread_mutex_unlock: EPERM when the caller does not hold it */
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return __libc_pthread_mutex_unlock(mutex);
}
