/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutexattr_getprotocol.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_mutexattr_getprotocol */
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict attr, int *restrict value)
{
	*value = attr->__protocol;

	return 0;
}
