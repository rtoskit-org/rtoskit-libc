/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_setschedprio.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/* POSIX pthread_setschedprio: a new priority under the thread's policy */
int pthread_setschedprio(pthread_t thread, int prio)
{
	int policy;
	int priority;
	int status = __rtos_thread_get_sched(thread, &policy, &priority);

	if (status < 0)
		return -status;

	return -__rtos_thread_set_sched(thread, policy, prio);
}
