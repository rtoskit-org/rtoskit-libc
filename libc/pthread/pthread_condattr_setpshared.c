/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_condattr_setpshared.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_condattr_setpshared: objects are private to the program (PLAN D148) */
int pthread_condattr_setpshared(pthread_condattr_t *attr, int value)
{
	if (value == PTHREAD_PROCESS_SHARED)
		return ENOTSUP;
	if (value != PTHREAD_PROCESS_PRIVATE)
		return EINVAL;
	attr->__pshared = value;

	return 0;
}
