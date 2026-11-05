/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cnd_timedwait.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <threads.h>

/* C23 7.28.3.5: ts is an absolute TIME_UTC time, the condition variable's CLOCK_REALTIME */
int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx, const struct timespec *restrict ts)
{
	int status = pthread_cond_timedwait(&cond->__cond, &mtx->__mutex, ts);

	if (status == ETIMEDOUT)
		return thrd_timedout;

	return status == 0 ? thrd_success : thrd_error;
}
