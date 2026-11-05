/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_setspecific.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

#include "internal/threads.h"

/* POSIX pthread_setspecific: EINVAL for a key not in use */
int pthread_setspecific(pthread_key_t key, const void *value)
{
	unsigned int generation;

	if (key >= __LIBC_TSS_MAX)
		return EINVAL;
	generation = __atomic_load_n(&__libc_tss_keys[key].generation, __ATOMIC_ACQUIRE);
	if ((generation & 1) == 0)
		return EINVAL;

	__libc_tss_values[key] = (struct __libc_tss_value){ .value = (void *)value, .generation = generation };

	return 0;
}
