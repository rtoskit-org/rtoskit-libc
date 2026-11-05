/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_spin_unlock.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_spin_unlock */
int pthread_spin_unlock(pthread_spinlock_t *lock)
{
	__atomic_store_n(lock, 0, __ATOMIC_RELEASE);

	return 0;
}
