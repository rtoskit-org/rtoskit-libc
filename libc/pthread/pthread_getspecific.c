/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_getspecific.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

#include "internal/threads.h"

/* POSIX pthread_getspecific: null for a key not in use, or a value set under an older generation of the key */
void *pthread_getspecific(pthread_key_t key)
{
	if (key >= __LIBC_TSS_MAX || __libc_tss_values[key].generation != __atomic_load_n(&__libc_tss_keys[key].generation, __ATOMIC_ACQUIRE))
		return nullptr;

	return __libc_tss_values[key].value;
}
