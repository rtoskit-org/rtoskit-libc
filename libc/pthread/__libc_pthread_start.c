/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_pthread_start.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "internal/threads.h"

/*
 * The first function of every thread (P10.03, P10.07): returning from a
 * POSIX routine is pthread_exit with its value, and from a C11 one
 * thrd_exit with its int
 */
void *__libc_pthread_start(void *start)
{
	struct __libc_pthread_start record = *(struct __libc_pthread_start *)start;

	free(start);
	if (record.c11 != nullptr)
		__libc_thread_exit((void *)(intptr_t)record.c11(record.arg));

	__libc_thread_exit(record.func(record.arg));
}
