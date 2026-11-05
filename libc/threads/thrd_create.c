/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * thrd_create.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdlib.h>
#include <threads.h>

#include "internal/threads.h"

/* C23 7.28.5.1: the new thread starts in __libc_thread_start with func and arg */
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct __libc_thread_start *start = malloc(sizeof(*start));
	int status;

	if (start == nullptr)
		return thrd_nomem;
	start->func = func;
	start->arg = arg;

	__atomic_fetch_add(&__libc_thread_count, 1, __ATOMIC_RELAXED);
	status = __rtos_thread_create(thr, nullptr, __libc_thread_start, start);
	if (status < 0) {
		__atomic_fetch_sub(&__libc_thread_count, 1, __ATOMIC_RELAXED);
		free(start);
		return status == -ENOMEM ? thrd_nomem : thrd_error;
	}

	return thrd_success;
}
