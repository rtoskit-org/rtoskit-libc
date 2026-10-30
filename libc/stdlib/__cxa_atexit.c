/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __cxa_atexit.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdlib.h>
#include <rtos/rtos-port.h>

#include "internal/cxa.h"
#include "internal/stdlib.h"

static struct __libc_cxa_block __libc_cxa_first;
struct __libc_cxa_block *__libc_cxa_newest = &__libc_cxa_first;

/*
 * The atexit table entry for a run of __cxa_atexit registrations. The exit
 * runner pops it from slot k, leaving the table's count at k, and the
 * entries registered while it was the newest slot carry level k: they are
 * the newest entries now, since those of every later slot ran first, so this
 * pops them newest first. Destructors and atexit handlers therefore run in
 * the reverse order of their registration, interleaved (C23 7.24.4.4p3,
 * C++ [basic.start.term]p3), and a run of C++ registrations takes one atexit
 * slot. Entries __cxa_finalize already ran are holes, popped on the way.
 */
static void __libc_cxa_run(void)
{
	unsigned int level;

	__rtos_lock_acquire(&__rtos_atexit_lock);
	level = __libc_atexit_handlers.count;
	__rtos_lock_release(&__rtos_atexit_lock);

	while (true) {
		struct __libc_cxa_entry entry = {};

		__rtos_lock_acquire(&__rtos_atexit_lock);
		while (true) {
			struct __libc_cxa_block *block = __libc_cxa_newest;

			if (block->count == 0) {
				if (block->older == nullptr)
					break;
				__libc_cxa_newest = block->older;
				free(block);
				continue;
			}
			if (block->entries[block->count - 1].func != nullptr && block->entries[block->count - 1].level != level)
				break;
			entry = block->entries[--block->count];
			if (entry.func != nullptr)
				break;
		}
		__rtos_lock_release(&__rtos_atexit_lock);

		if (entry.func == nullptr)
			return;
		entry.func(entry.arg);
	}
}

/*
 * Itanium C++ ABI 3.3.5.3: register func(arg) to run at exit, or when
 * __cxa_finalize(dso) runs; 0 on success. A new block comes from malloc
 * outside the lock; the attempt is retried, since another thread may have
 * made room meanwhile.
 */
int __cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	struct __libc_cxa_block *spare = nullptr;
	int status = -1;

	while (true) {
		struct __libc_exit_handlers *table = &__libc_atexit_handlers;
		bool need_block = false;

		__rtos_lock_acquire(&__rtos_atexit_lock);
		struct __libc_cxa_block *block = __libc_cxa_newest;

		if (block->count == __LIBC_CXA_BLOCK_ENTRIES) {
			if (spare != nullptr) {
				spare->older = block;
				__libc_cxa_newest = spare;
				block = spare;
				spare = nullptr;
			} else {
				need_block = true;
			}
		}
		if (!need_block) {
			if (table->count > 0 && table->handlers[table->count - 1] == __libc_cxa_run) {
				block->entries[block->count++] = (struct __libc_cxa_entry){ .func = func, .arg = arg, .dso = dso, .level = table->count - 1 };
				status = 0;
			} else if (table->count < __LIBC_EXIT_HANDLERS) {
				table->handlers[table->count++] = __libc_cxa_run;
				block->entries[block->count++] = (struct __libc_cxa_entry){ .func = func, .arg = arg, .dso = dso, .level = table->count - 1 };
				status = 0;
			}
		}
		__rtos_lock_release(&__rtos_atexit_lock);

		if (!need_block)
			break;

		spare = calloc(1, sizeof(*spare));
		if (spare == nullptr)
			break;
	}

	free(spare);
	return status;
}
