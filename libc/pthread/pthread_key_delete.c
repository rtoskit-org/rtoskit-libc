/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_key_delete.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

#include "internal/threads.h"

/* POSIX pthread_key_delete: no destructor runs; the next generation is free and invalidates every value; EINVAL for a key not in use */
int pthread_key_delete(pthread_key_t key)
{
	unsigned int generation;

	if (key >= __LIBC_TSS_MAX)
		return EINVAL;
	generation = __atomic_load_n(&__libc_tss_keys[key].generation, __ATOMIC_ACQUIRE);
	if ((generation & 1) == 0)
		return EINVAL;

	__atomic_store_n(&__libc_tss_keys[key].dtor, nullptr, __ATOMIC_RELAXED);
	return __atomic_compare_exchange_n(&__libc_tss_keys[key].generation, &generation, generation + 1, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED) ? 0 : EINVAL;
}
