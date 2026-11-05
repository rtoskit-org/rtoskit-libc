/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_lock_acquire.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * The seam's lock word (rtos-port.h, PLAN D79) on a futex. One compare-and-
 * swap either takes a free lock or marks a held one slow. The owner counts
 * nested acquisitions in depth and marks the word slow so that its releases
 * come here: a plain store, since anyone else only ever sets that same bit.
 * Another thread marks the word and sleeps until it finds it free, then
 * takes it still marked, since more threads may be waiting.
 */
void __rtos_lock_acquire(struct __rtos_lock *lock)
{
	unsigned int self = __rtos_lock_token;
	unsigned int word = 0;

	while (true) {
		/* The caller's own token, marked or not; after a wait, self carries the mark too */
		if ((word | __RTOS_LOCK_SLOW) == (self | __RTOS_LOCK_SLOW)) {
			++lock->depth;
			__atomic_store_n(&lock->word, self | __RTOS_LOCK_SLOW, __ATOMIC_RELAXED);
			return;
		}

		unsigned int want = word == 0 ? self : word | __RTOS_LOCK_SLOW;

		if (__atomic_compare_exchange_n(&lock->word, &word, want, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
			if (word == 0)
				return;
			__rtos_futex_wait(&lock->word, want, TIME_UTC, nullptr);
			self |= __RTOS_LOCK_SLOW;
			word = 0;
		}
	}
}
