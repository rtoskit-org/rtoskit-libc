/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * mtx_timedlock.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <threads.h>

/* C23 7.28.4.4: ts is an absolute TIME_UTC time, as pthread_mutex_timedlock's CLOCK_REALTIME; only for a timed mutex */
int mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts)
{
	int status;

	if ((mtx->__type & mtx_timed) == 0)
		return thrd_error;

	status = pthread_mutex_timedlock(&mtx->__mutex, ts);
	if (status == ETIMEDOUT)
		return thrd_timedout;

	return status == 0 ? thrd_success : thrd_error;
}
