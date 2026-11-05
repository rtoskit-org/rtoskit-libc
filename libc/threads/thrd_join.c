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

#include <rtos/rtos-port.h>
#include <threads.h>

/* C23 7.28.5.6 */
int thrd_join(thrd_t thr, int *res)
{
	void *result;

	if (__rtos_thread_join(thr, &result) < 0)
		return thrd_error;
	/* thrd_exit's int travels through the seam's void * (P10.02) */
	if (res != nullptr)
		*res = (int)(intptr_t)result;

	return thrd_success;
}
