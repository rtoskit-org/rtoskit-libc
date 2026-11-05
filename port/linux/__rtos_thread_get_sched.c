/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_get_sched.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

/* The policy and priority the thread was given (P10.02): the test port records them only */
int __rtos_thread_get_sched(struct __rtos_thread *thread, int *policy, int *priority)
{
	*policy = __atomic_load_n(&thread->policy, __ATOMIC_RELAXED);
	*priority = __atomic_load_n(&thread->priority, __ATOMIC_RELAXED);

	return 0;
}
