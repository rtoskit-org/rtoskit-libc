/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_set_sched.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * Record a new policy and priority (P10.02, PLAN D149). The test port runs
 * unprivileged, where Linux refuses the real-time policies, so it checks and
 * records them without asking the kernel; an RTOS port applies them.
 */
int __rtos_thread_set_sched(struct __rtos_thread *thread, int policy, int priority)
{
	int min;
	int max;
	int status = __rtos_sched_priority_range(policy, &min, &max);

	if (status < 0)
		return status;
	if (priority < min || priority > max)
		return -EINVAL;

	__atomic_store_n(&thread->policy, policy, __ATOMIC_RELAXED);
	__atomic_store_n(&thread->priority, priority, __ATOMIC_RELAXED);

	return 0;
}
