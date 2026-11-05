/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * thrd_join.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <stdint.h>
#include <threads.h>

/* C23 7.28.5.6 over pthread_join; thrd_exit's int comes back as the thread's value (P10.07) */
int thrd_join(thrd_t thr, int *res)
{
	void *value;

	if (pthread_join(thr, &value) != 0)
		return thrd_error;
	if (res != nullptr)
		*res = (int)(intptr_t)value;

	return thrd_success;
}
