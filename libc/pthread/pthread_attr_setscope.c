/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_setscope.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

/* POSIX pthread_attr_setscope: every thread competes system-wide, so PTHREAD_SCOPE_PROCESS is ENOTSUP */
int pthread_attr_setscope(pthread_attr_t *attr, int value)
{
	if (value == PTHREAD_SCOPE_PROCESS)
		return ENOTSUP;
	if (value != PTHREAD_SCOPE_SYSTEM)
		return EINVAL;
	attr->__scope = value;

	return 0;
}
