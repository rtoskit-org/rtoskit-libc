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
#include <time.h>

#include "rtos-none.h"

/*
 * With one thread only an interrupt handler can change the value: spin until
 * it does, or until the deadline passes on the board's clock. Without a
 * clock a deadline cannot be waited for and has passed at once. The
 * deadline is on the time base base (P10.02, PLAN D150).
 */
int __rtos_futex_wait(unsigned int *address, unsigned int expected, int base, const struct timespec *deadline)
{
	struct timespec now;

	if (base != TIME_UTC && base != TIME_MONOTONIC)
		return -EINVAL;
	if (deadline != nullptr && (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000))
		return -EINVAL;
	if (__atomic_load_n(address, __ATOMIC_ACQUIRE) != expected)
		return -EAGAIN;

	while (__atomic_load_n(address, __ATOMIC_ACQUIRE) == expected) {
		if (deadline == nullptr)
			continue;
		if (__rtos_clock_gettime(base, &now) < 0 || now.tv_sec > deadline->tv_sec || (now.tv_sec == deadline->tv_sec && now.tv_nsec >= deadline->tv_nsec))
			return -ETIMEDOUT;
	}

	return 0;
}
