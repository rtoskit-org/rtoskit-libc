/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_spin_init.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_spin_init: unlocked; objects are private to the program (PLAN D148) */
int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	if (pshared == PTHREAD_PROCESS_SHARED)
		return ENOTSUP;
	if (pshared != PTHREAD_PROCESS_PRIVATE)
		return EINVAL;
	*lock = 0;

	return 0;
}
