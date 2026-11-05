/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_create.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "rtos-none.h"

/* One thread only: thrd_create reports thrd_error */
int __rtos_thread_create(struct __rtos_thread **, const struct __rtos_thread_attr *, void *(*)(void *), void *)
{
	return -ENOSYS;
}
