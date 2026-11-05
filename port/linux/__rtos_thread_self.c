/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_self.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

struct __rtos_thread __rtos_linux_main_thread = { .state = __LINUX_THREAD_JOINABLE, .policy = SCHED_OTHER, .priority = 0 };
thread_local struct __rtos_thread *__rtos_linux_self;

/* New threads set their record at start; the initial thread has none in its TLS */
struct __rtos_thread *__rtos_thread_self(void)
{
	struct __rtos_thread *self = __rtos_linux_self;

	return self != nullptr ? self : &__rtos_linux_main_thread;
}
