/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_mutex_lock.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <rtos/rtos-port.h>
#include <threads.h>

#include "internal/threads.h"

/*
 * The three-state futex mutex (docs/research/threads.md): 0 -> 1 when free;
 * otherwise mark it contended (2) and sleep until an exchange finds it free.
 * A recursive mutex held by the caller only counts.
 */
int __libc_mutex_lock(mtx_t *mtx, const struct timespec *deadline)
{
	thrd_t self = __rtos_thread_self();
	unsigned int state = __LIBC_MUTEX_UNLOCKED;

	if ((mtx->__type & mtx_recursive) != 0 && __atomic_load_n(&mtx->__owner, __ATOMIC_RELAXED) == self) {
		if (mtx->__count == UINT_MAX)
			return thrd_error;
		++mtx->__count;
		return thrd_success;
	}

	if (!__atomic_compare_exchange_n(&mtx->__state, &state, __LIBC_MUTEX_LOCKED, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
		while (__atomic_exchange_n(&mtx->__state, __LIBC_MUTEX_CONTENDED, __ATOMIC_ACQUIRE) != __LIBC_MUTEX_UNLOCKED) {
			int status = __rtos_futex_wait(&mtx->__state, __LIBC_MUTEX_CONTENDED, TIME_UTC, deadline);

			if (status == -ETIMEDOUT)
				return thrd_timedout;
			if (status == -EINVAL)
				return thrd_error;
		}
	}

	__atomic_store_n(&mtx->__owner, self, __ATOMIC_RELAXED);
	mtx->__count = 1;

	return thrd_success;
}
