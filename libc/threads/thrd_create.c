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
#include <stdlib.h>
#include <threads.h>

#include "internal/threads.h"

/* C23 7.28.5.1 over pthreads (P10.07): thrd_nomem when memory runs out, thrd_error when the port cannot make a thread */
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct __libc_pthread_start *start = malloc(sizeof(*start));
	int status;

	if (start == nullptr)
		return thrd_nomem;
	*start = (struct __libc_pthread_start){ .func = nullptr, .c11 = func, .arg = arg };

	status = __libc_pthread_create(thr, nullptr, start);
	if (status < 0)
		return status == -ENOMEM ? thrd_nomem : thrd_error;

	return thrd_success;
}
