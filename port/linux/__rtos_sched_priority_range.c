/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_sched_priority_range.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <sched.h>

#include "linux.h"

/* Linux's ranges (P10.02, PLAN D149): 1-99 for the real-time policies, 0 for SCHED_OTHER */
int __rtos_sched_priority_range(int policy, int *min, int *max)
{
	switch (policy) {
		case SCHED_FIFO:
		case SCHED_RR:
			*min = __LINUX_RT_PRIORITY_MIN;
			*max = __LINUX_RT_PRIORITY_MAX;
			return 0;
		case SCHED_OTHER:
			*min = 0;
			*max = 0;
			return 0;
		default:
			return -EINVAL;
	}
}
