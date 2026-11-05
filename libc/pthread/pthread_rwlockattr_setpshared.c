/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlockattr_setpshared.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_rwlockattr_setpshared: objects are private to the program (PLAN D148) */
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
	if (pshared == PTHREAD_PROCESS_SHARED)
		return ENOTSUP;
	if (pshared != PTHREAD_PROCESS_PRIVATE)
		return EINVAL;
	attr->__pshared = pshared;

	return 0;
}
