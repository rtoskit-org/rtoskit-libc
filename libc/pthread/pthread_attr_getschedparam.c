/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_getschedparam.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_attr_getschedparam */
int pthread_attr_getschedparam(const pthread_attr_t *restrict attr, struct sched_param *restrict param)
{
	param->sched_priority = attr->__priority;

	return 0;
}
