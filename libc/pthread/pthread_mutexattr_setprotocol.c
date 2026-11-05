/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutexattr_setprotocol.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * POSIX pthread_mutexattr_setprotocol: PTHREAD_PRIO_INHERIT where the port
 * has priority-inheritance locks (P10.05, PLAN D154), found by taking and
 * releasing a word of its own; ENOTSUP otherwise, and for
 * PTHREAD_PRIO_PROTECT
 */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int value)
{
	unsigned int word = 0;

	if (value == PTHREAD_PRIO_PROTECT)
		return ENOTSUP;
	if (value == PTHREAD_PRIO_INHERIT) {
		int status = __rtos_mutex_lock_pi(&word, TIME_UTC, nullptr);

		if (status < 0)
			return ENOTSUP;
		__rtos_mutex_unlock_pi(&word);
	} else if (value != PTHREAD_PRIO_NONE) {
		return EINVAL;
	}
	attr->__protocol = value;

	return 0;
}
