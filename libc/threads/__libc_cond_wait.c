/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_cond_wait.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <threads.h>

#include "internal/threads.h"

/*
 * C23 7.28.3.5, 7.28.3.6: read the sequence while holding the mutex, release
 * it, and sleep while the sequence is unchanged; a signal in between changes
 * it, so no wake is lost. A recursive mutex is released fully and its count
 * restored.
 */
int __libc_cond_wait(cnd_t *cond, mtx_t *mtx, const struct timespec *deadline)
{
	unsigned int sequence = __atomic_load_n(&cond->__sequence, __ATOMIC_RELAXED);
	unsigned int count = mtx->__count;
	int status;

	mtx->__count = 1;
	if (__libc_mutex_unlock(mtx) != thrd_success) {
		mtx->__count = count;
		return thrd_error;
	}

	status = __rtos_futex_wait(&cond->__sequence, sequence, TIME_UTC, deadline);

	while (__libc_mutex_lock(mtx, nullptr) != thrd_success)
		continue;
	mtx->__count = count;

	if (status == -ETIMEDOUT)
		return thrd_timedout;

	return status == -EINVAL ? thrd_error : thrd_success;
}
