/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setstack.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>

/* POSIX pthread_attr_setstack: the caller's stack, which stays the caller's to free; EINVAL below PTHREAD_STACK_MIN */
int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
	if (stacksize < PTHREAD_STACK_MIN)
		return EINVAL;
	attr->__stack = stackaddr;
	attr->__stack_size = stacksize;

	return 0;
}
