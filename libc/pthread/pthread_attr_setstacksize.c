/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setstacksize.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>

/* POSIX pthread_attr_setstacksize: EINVAL below PTHREAD_STACK_MIN */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t value)
{
	if (value < PTHREAD_STACK_MIN)
		return EINVAL;
	attr->__stack_size = value;

	return 0;
}
