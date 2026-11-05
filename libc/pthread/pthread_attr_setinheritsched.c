/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setinheritsched.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_attr_setinheritsched: EINVAL for anything but inherited or explicit */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int value)
{
	if (value != PTHREAD_INHERIT_SCHED && value != PTHREAD_EXPLICIT_SCHED)
		return EINVAL;
	attr->__inherit = value;

	return 0;
}
