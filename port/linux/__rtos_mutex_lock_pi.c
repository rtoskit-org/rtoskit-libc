/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_mutex_lock_pi.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * Linux's priority-inheritance futex (P10.05, PLAN D154): a free word (0)
 * takes the caller's tid without the kernel; otherwise FUTEX_LOCK_PI, with
 * an absolute deadline on CLOCK_REALTIME, or FUTEX_LOCK_PI2 on
 * CLOCK_MONOTONIC. The kernel boosts the owner while waiters sleep, and
 * gives EDEADLK when the caller already owns the word.
 */
int __rtos_mutex_lock_pi(unsigned int *word, int base, const struct timespec *deadline)
{
	unsigned int tid = (unsigned int)__rtos_linux_tid();
	unsigned int expected = 0;
	struct __linux_timespec timeout;
	long operation;
	long status;

	if (base != TIME_UTC && base != TIME_MONOTONIC)
		return -EINVAL;
	if (__atomic_compare_exchange_n(word, &expected, tid, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		return 0;
	if ((expected & __LINUX_FUTEX_TID_MASK) == tid)
		return -EDEADLK;
	if (deadline != nullptr) {
		if (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000)
			return -EINVAL;
		if (deadline->tv_sec < 0)
			return -ETIMEDOUT;
		timeout = (struct __linux_timespec){ .tv_sec = deadline->tv_sec, .tv_nsec = deadline->tv_nsec };
	}

	operation = (base == TIME_UTC ? __LINUX_FUTEX_LOCK_PI : __LINUX_FUTEX_LOCK_PI2) | __LINUX_FUTEX_PRIVATE;
	do
		status = __linux_syscall6(__LINUX_SYS_FUTEX, (long)word, operation, 0, deadline != nullptr ? (long)&timeout : 0, 0, 0);
	while (status == -EINTR);

	return (int)status;
}
