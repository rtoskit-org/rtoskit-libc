/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_cond_broadcast.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>

/* POSIX pthread_cond_broadcast: a new sequence number, and every waiter woken */
int pthread_cond_broadcast(pthread_cond_t *cond)
{
	__atomic_fetch_add(&cond->__sequence, 1, __ATOMIC_SEQ_CST);
	__rtos_futex_wake(&cond->__sequence, UINT_MAX);

	return 0;
}
