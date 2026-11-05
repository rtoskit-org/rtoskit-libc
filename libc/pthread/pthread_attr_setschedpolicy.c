/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setschedpolicy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/* POSIX pthread_attr_setschedpolicy: ENOTSUP for a policy the port does not have (PLAN D149) */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	int min;
	int max;

	if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
		return EINVAL;
	if (__rtos_sched_priority_range(policy, &min, &max) < 0)
		return ENOTSUP;
	attr->__policy = policy;

	return 0;
}
