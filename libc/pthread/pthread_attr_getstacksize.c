/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_getstacksize.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>
#include <stddef.h>

/* POSIX pthread_attr_getstacksize */
int pthread_attr_getstacksize(const pthread_attr_t *restrict attr, size_t *restrict value)
{
	*value = attr->__stack_size;

	return 0;
}
