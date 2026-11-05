/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_mutex_init.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/* POSIX pthread_mutex_init: the attribute's type and protocol, or a default mutex */
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr)
{
	*mutex = (pthread_mutex_t){ .__state = 0, .__type = attr != nullptr ? attr->__type : PTHREAD_MUTEX_DEFAULT, .__owner = nullptr, .__count = 0, .__protocol = attr != nullptr ? attr->__protocol : PTHREAD_PRIO_NONE };

	return 0;
}
