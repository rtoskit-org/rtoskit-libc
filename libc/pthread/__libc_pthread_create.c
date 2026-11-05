/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_create.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>
#include <stdlib.h>

#include "internal/threads.h"

/*
 * Thread creation for pthread_create and thrd_create (P10.03, P10.07): attr
 * becomes the seam's attribute record (null inherits the creator's
 * scheduling), and a thread created detached is detached as soon as it
 * exists. start is the new thread's, or freed here on failure.
 */
int __libc_pthread_create(pthread_t *thread, const pthread_attr_t *attr, struct __libc_pthread_start *start)
{
	struct __rtos_thread_attr settings;
	int status;

	if (attr != nullptr)
		settings = (struct __rtos_thread_attr){ .stack_size = attr->__stack_size, .stack = attr->__stack, .policy = attr->__policy, .priority = attr->__priority, .inherit = attr->__inherit == PTHREAD_INHERIT_SCHED };

	__atomic_fetch_add(&__libc_thread_count, 1, __ATOMIC_RELAXED);
	status = __rtos_thread_create(thread, attr != nullptr ? &settings : nullptr, __libc_pthread_start, start);
	if (status < 0) {
		__atomic_fetch_sub(&__libc_thread_count, 1, __ATOMIC_RELAXED);
		free(start);
		return status;
	}

	if (attr != nullptr && attr->__detach == PTHREAD_CREATE_DETACHED)
		__rtos_thread_detach(*thread);

	return 0;
}
