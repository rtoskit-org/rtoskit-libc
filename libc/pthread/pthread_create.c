/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_create.c
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
 * POSIX pthread_create over the seam (P10.03): attr becomes the seam's
 * attribute record (a null attr inherits the creator's scheduling), and a
 * detached thread is detached as soon as it exists. EAGAIN when memory or
 * the port's threads run out (port/none has none), EINVAL for a priority
 * outside the policy's range, EPERM where the port refuses the setting.
 */
int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
{
	struct __rtos_thread_attr settings;
	struct __libc_pthread_start *start = malloc(sizeof(*start));
	int status;

	if (start == nullptr)
		return EAGAIN;
	start->func = start_routine;
	start->arg = arg;

	if (attr != nullptr)
		settings = (struct __rtos_thread_attr){ .stack_size = attr->__stack_size, .stack = attr->__stack, .policy = attr->__policy, .priority = attr->__priority, .inherit = attr->__inherit == PTHREAD_INHERIT_SCHED };

	__atomic_fetch_add(&__libc_thread_count, 1, __ATOMIC_RELAXED);
	status = __rtos_thread_create(thread, attr != nullptr ? &settings : nullptr, __libc_pthread_start, start);
	if (status < 0) {
		__atomic_fetch_sub(&__libc_thread_count, 1, __ATOMIC_RELAXED);
		free(start);
		return status == -EINVAL || status == -EPERM ? -status : EAGAIN;
	}

	if (attr != nullptr && attr->__detach == PTHREAD_CREATE_DETACHED)
		__rtos_thread_detach(*thread);

	return 0;
}
