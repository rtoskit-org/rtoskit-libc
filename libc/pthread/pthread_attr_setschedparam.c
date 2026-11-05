/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setschedparam.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_attr_setschedparam: pthread_create checks the priority against the policy's range */
int pthread_attr_setschedparam(pthread_attr_t *restrict attr, const struct sched_param *restrict param)
{
	attr->__priority = param->sched_priority;

	return 0;
}
