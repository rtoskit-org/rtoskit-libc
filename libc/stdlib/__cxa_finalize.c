/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __cxa_finalize.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "internal/cxa.h"
#include "internal/stdlib.h"

/*
 * Itanium C++ ABI 3.3.5.4. With a null dso every termination function runs,
 * the atexit handlers included, as exit would run them. Otherwise the
 * destructors registered for that dso run, newest first, each taken out of
 * its block first (it becomes a hole the exit runner skips), so none runs
 * twice and one may register another.
 */
void __cxa_finalize(void *dso)
{
	if (dso == nullptr) {
		__libc_atexit_run();
		return;
	}

	while (true) {
		struct __libc_cxa_entry entry = {};

		__rtos_lock_acquire(&__rtos_atexit_lock);
		for (struct __libc_cxa_block *block = __libc_cxa_newest; block != nullptr && entry.func == nullptr; block = block->older) {
			for (unsigned int index = block->count; index > 0; --index) {
				struct __libc_cxa_entry *candidate = &block->entries[index - 1];

				if (candidate->func != nullptr && candidate->dso == dso) {
					entry = *candidate;
					candidate->func = nullptr;
					break;
				}
			}
		}
		__rtos_lock_release(&__rtos_atexit_lock);

		if (entry.func == nullptr)
			return;
		entry.func(entry.arg);
	}
}
