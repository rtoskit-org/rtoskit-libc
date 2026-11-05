/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setdetachstate.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_attr_setdetachstate: EINVAL for anything but joinable or detached */
int pthread_attr_setdetachstate(pthread_attr_t *attr, int value)
{
	if (value != PTHREAD_CREATE_JOINABLE && value != PTHREAD_CREATE_DETACHED)
		return EINVAL;
	attr->__detach = value;

	return 0;
}
