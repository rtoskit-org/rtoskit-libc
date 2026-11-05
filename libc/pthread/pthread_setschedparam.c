/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_setschedparam.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/*
 * POSIX pthread_setschedparam (PLAN D149): EINVAL for a policy the port
 * lacks or a priority outside its range, EPERM where the port refuses
 */
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
	return -__rtos_thread_set_sched(thread, policy, param->sched_priority);
}
