/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sched_get_priority_max.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <sched.h>

/* POSIX sched_get_priority_max: the top of the policy's range, or -1 with EINVAL (PLAN D149) */
int sched_get_priority_max(int policy)
{
	int min;
	int max;
	int status = __rtos_sched_priority_range(policy, &min, &max);

	if (status < 0) {
		errno = -status;
		return -1;
	}

	return max;
}
