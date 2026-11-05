/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * mtx_trylock.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>
#include <threads.h>

/* C23 7.28.4.5 over pthread_mutex_trylock */
int mtx_trylock(mtx_t *mtx)
{
	int status = pthread_mutex_trylock(&mtx->__mutex);

	if (status == EBUSY)
		return thrd_busy;

	return status == 0 ? thrd_success : thrd_error;
}
