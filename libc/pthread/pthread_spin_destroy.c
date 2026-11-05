/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_spin_destroy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_spin_destroy: EBUSY while it is held */
int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	return __atomic_load_n(lock, __ATOMIC_RELAXED) != 0 ? EBUSY : 0;
}
