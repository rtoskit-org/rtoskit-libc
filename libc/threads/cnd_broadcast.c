/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cnd_broadcast.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <threads.h>

/* C23 7.28.3.1 over pthread_cond_broadcast */
int cnd_broadcast(cnd_t *cond)
{
	return pthread_cond_broadcast(&cond->__cond) == 0 ? thrd_success : thrd_error;
}
