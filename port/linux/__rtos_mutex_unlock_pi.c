/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_mutex_unlock_pi.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * A word that holds only the caller's tid is freed without the kernel; one
 * with waiters goes through FUTEX_UNLOCK_PI, which hands it to the most
 * urgent waiter and ends the owner's boost (P10.05)
 */
int __rtos_mutex_unlock_pi(unsigned int *word)
{
	unsigned int tid = (unsigned int)__rtos_linux_tid();
	unsigned int expected = tid;

	if ((__atomic_load_n(word, __ATOMIC_RELAXED) & __LINUX_FUTEX_TID_MASK) != tid)
		return -EPERM;
	if (__atomic_compare_exchange_n(word, &expected, 0, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED))
		return 0;

	return (int)__linux_syscall6(__LINUX_SYS_FUTEX, (long)word, __LINUX_FUTEX_UNLOCK_PI | __LINUX_FUTEX_PRIVATE, 0, 0, 0, 0);
}
