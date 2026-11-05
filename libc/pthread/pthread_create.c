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
#include <stdlib.h>

#include "internal/threads.h"

/*
 * POSIX pthread_create over the seam (P10.03): EAGAIN when memory or the
 * port's threads run out (port/none has none), EINVAL for a priority
 * outside the policy's range, EPERM where the port refuses the setting
 */
int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
{
	struct __libc_pthread_start *start = malloc(sizeof(*start));
	int status;

	if (start == nullptr)
		return EAGAIN;
	*start = (struct __libc_pthread_start){ .func = start_routine, .c11 = nullptr, .arg = arg };

	status = __libc_pthread_create(thread, attr, start);
	if (status < 0)
		return status == -EINVAL || status == -EPERM ? -status : EAGAIN;

	return 0;
}
