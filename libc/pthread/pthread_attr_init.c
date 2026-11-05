/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_attr_init.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

/*
 * POSIX pthread_attr_init: the port's stack, joinable, the creator's
 * scheduling (PTHREAD_INHERIT_SCHED), SCHED_OTHER at 0 when explicit
 */
int pthread_attr_init(pthread_attr_t *attr)
{
	*attr = (pthread_attr_t){ .__stack_size = 0, .__stack = nullptr, .__policy = SCHED_OTHER, .__priority = 0, .__inherit = PTHREAD_INHERIT_SCHED, .__detach = PTHREAD_CREATE_JOINABLE, .__scope = PTHREAD_SCOPE_SYSTEM };

	return 0;
}
