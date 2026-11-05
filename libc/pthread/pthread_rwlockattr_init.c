/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_rwlockattr_init.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_rwlockattr_init: private to the program */
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	attr->__pshared = PTHREAD_PROCESS_PRIVATE;

	return 0;
}
