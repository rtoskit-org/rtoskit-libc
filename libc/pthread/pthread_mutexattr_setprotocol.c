/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutexattr_setprotocol.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_mutexattr_setprotocol: priority inheritance comes with P10.05; priority protection is not supported */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int value)
{
	if (value == PTHREAD_PRIO_INHERIT || value == PTHREAD_PRIO_PROTECT)
		return ENOTSUP;
	if (value != PTHREAD_PRIO_NONE)
		return EINVAL;
	attr->__protocol = value;

	return 0;
}
