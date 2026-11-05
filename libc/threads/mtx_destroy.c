/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * mtx_destroy.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <threads.h>

/* C23 7.28.4.3: a mutex holds no resources */
void mtx_destroy(mtx_t *mtx)
{
	pthread_mutex_destroy(&mtx->__mutex);
	mtx->__type = -1;
}
