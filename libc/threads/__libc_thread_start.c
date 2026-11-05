/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_thread_start.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdlib.h>
#include <threads.h>

#include "internal/threads.h"

/* C23 7.28.5.1p3: returning from func is thrd_exit with its result */
void *__libc_thread_start(void *start)
{
	struct __libc_thread_start record = *(struct __libc_thread_start *)start;

	free(start);
	thrd_exit(record.func(record.arg));
}
