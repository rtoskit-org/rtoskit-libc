/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutexattr_settype.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_mutexattr_settype: normal (the default), recursive or error-checking */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int value)
{
	if (value != PTHREAD_MUTEX_NORMAL && value != PTHREAD_MUTEX_RECURSIVE && value != PTHREAD_MUTEX_ERRORCHECK)
		return EINVAL;
	attr->__type = value;

	return 0;
}
