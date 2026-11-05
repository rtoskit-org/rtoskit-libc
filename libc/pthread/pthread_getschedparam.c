/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_getschedparam.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/* POSIX pthread_getschedparam: the thread's policy and priority, in the port's POSIX numbers (PLAN D149) */
int pthread_getschedparam(pthread_t thread, int *restrict policy, struct sched_param *restrict param)
{
	int priority;
	int status = __rtos_thread_get_sched(thread, policy, &priority);

	if (status < 0)
		return -status;
	param->sched_priority = priority;

	return 0;
}
