/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_barrier_init.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_barrier_init: EINVAL for a count of 0 (attr only says the barrier is private) */
int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict, unsigned int count)
{
	if (count == 0)
		return EINVAL;

	*barrier = (pthread_barrier_t){ .__count = count, .__arrived = 0, .__generation = 0 };

	return 0;
}
