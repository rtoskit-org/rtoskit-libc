/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_join.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/* POSIX pthread_join: EDEADLK for the calling thread, EINVAL for one that is not joinable */
int pthread_join(pthread_t thread, void **value_ptr)
{
	void *value;

	if (thread == __rtos_thread_self())
		return EDEADLK;
	if (__rtos_thread_join(thread, &value) < 0)
		return EINVAL;
	if (value_ptr != nullptr)
		*value_ptr = value;

	return 0;
}
