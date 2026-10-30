/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __cxa_thread_atexit.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdlib.h>

#include "internal/cxa.h"
#include "internal/stdlib.h"

/*
 * The C++ ABI hook for thread_local objects with destructors (what
 * libstdc++'s configure looks for in the C library before falling back to
 * its own): func(obj) runs when the calling thread ends, newest first, before
 * its tss destructors (PLAN D121). The node comes from malloc; 0 on success.
 */
/* exit runs the calling thread's list from the atexit runner: link it (a weak reference alone would not) */
[[gnu::used]] static struct __libc_exit_handlers *const __libc_cxa_thread_exit_link = &__libc_atexit_handlers;

int __cxa_thread_atexit(void (*func)(void *), void *obj, [[maybe_unused]] void *dso)
{
	struct __libc_cxa_thread_entry *entry = malloc(sizeof(*entry));

	if (entry == nullptr)
		return -1;

	*entry = (struct __libc_cxa_thread_entry){ .next = __libc_cxa_thread_entries, .func = func, .obj = obj };
	__libc_cxa_thread_entries = entry;

	return 0;
}
