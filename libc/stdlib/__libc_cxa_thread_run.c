/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_cxa_thread_run.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdlib.h>

#include "internal/cxa.h"
#include "internal/stdlib.h"

/* The calling thread's destructors (PLAN D6: linked only with __cxa_thread_atexit) */
thread_local struct __libc_cxa_thread_entry *__libc_cxa_thread_entries;

/*
 * Each entry is unlinked before its destructor runs, so a destructor that
 * registers another (a thread_local first used in a destructor) has it run
 * next.
 */
void __libc_cxa_thread_run(void)
{
	while (__libc_cxa_thread_entries != nullptr) {
		struct __libc_cxa_thread_entry *entry = __libc_cxa_thread_entries;

		__libc_cxa_thread_entries = entry->next;
		entry->func(entry->obj);
		free(entry);
	}
}
