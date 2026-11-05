/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_futex_wait.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * A private futex wait; with a deadline, FUTEX_WAIT_BITSET takes it as an
 * absolute time, on CLOCK_REALTIME for TIME_UTC (FUTEX_CLOCK_REALTIME) and
 * on CLOCK_MONOTONIC otherwise (P10.02, PLAN D150). An interrupted wait is
 * an early return (0). A deadline before the clock's zero has passed.
 */
int __rtos_futex_wait(unsigned int *address, unsigned int expected, int base, const struct timespec *deadline)
{
	struct __linux_timespec timeout;
	long operation = __LINUX_FUTEX_WAIT | __LINUX_FUTEX_PRIVATE;
	long status;

	if (base != TIME_UTC && base != TIME_MONOTONIC)
		return -EINVAL;
	if (deadline != nullptr) {
		if (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000)
			return -EINVAL;
		if (deadline->tv_sec < 0)
			return -ETIMEDOUT;
		timeout = (struct __linux_timespec){ .tv_sec = deadline->tv_sec, .tv_nsec = deadline->tv_nsec };
		operation = __LINUX_FUTEX_WAIT_BITSET | __LINUX_FUTEX_PRIVATE | (base == TIME_UTC ? __LINUX_FUTEX_CLOCK_REALTIME : 0);
	}

	status = __linux_syscall6(__LINUX_SYS_FUTEX, (long)address, operation, (long)expected, deadline != nullptr ? (long)&timeout : 0, 0, (long)__LINUX_FUTEX_BITSET_ANY);
	if (status == -EINTR)
		return 0;

	return status < 0 ? (int)status : 0;
}
