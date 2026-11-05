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
#include <sched.h>

/* Only the one setting there is: SCHED_OTHER at priority 0 */
int __rtos_thread_set_sched(struct __rtos_thread *, int policy, int priority)
{
	if (policy != SCHED_OTHER || priority != 0)
		return -EINVAL;

	return 0;
}
