/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_key_create.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <pthread.h>

#include "internal/threads.h"

/*
 * POSIX pthread_key_create on <threads.h>'s key table (PLAN D153): claim a
 * free key (even generation) by making its generation odd; EAGAIN when all
 * PTHREAD_KEYS_MAX are in use
 */
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	for (unsigned int index = 0; index < __LIBC_TSS_MAX; ++index) {
		unsigned int generation = __atomic_load_n(&__libc_tss_keys[index].generation, __ATOMIC_RELAXED);

		if ((generation & 1) == 0 && __atomic_compare_exchange_n(&__libc_tss_keys[index].generation, &generation, generation + 1, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
			__atomic_store_n(&__libc_tss_keys[index].dtor, destructor, __ATOMIC_RELEASE);
			*key = index;
			return 0;
		}
	}

	return EAGAIN;
}
