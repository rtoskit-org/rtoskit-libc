/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * mtx_init.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <threads.h>

/* C23 7.28.4.2: plain or timed, optionally recursive; a POSIX normal or recursive mutex underneath (P10.07) */
int mtx_init(mtx_t *mtx, int type)
{
	pthread_mutexattr_t attr;

	if (type != mtx_plain && type != mtx_timed && type != (mtx_plain | mtx_recursive) && type != (mtx_timed | mtx_recursive))
		return thrd_error;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, (type & mtx_recursive) != 0 ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&mtx->__mutex, &attr);
	mtx->__type = type;

	return thrd_success;
}
