/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * once.h
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_ONCE_H
#define _INTERNAL_ONCE_H

#include <limits.h>
#include <rtos/rtos-port.h>
#include <threads.h>

/*
 * The once_flag state machine shared by call_once and __libc_call_once_eh
 * (docs/research/threads.md, PLAN D145).
 */
#define __LIBC_ONCE_NOT_RUN 0u
#define __LIBC_ONCE_RUNNING 1u
#define __LIBC_ONCE_WAITING 2u
#define __LIBC_ONCE_DONE 3u

/*
 * C23 7.28.2.1: returns true when the caller moved the flag from NOT_RUN to
 * RUNNING and must run the function, false once the flag is DONE. Others
 * sleep while it runs; a flag that went back to NOT_RUN (an exception left
 * the function in __libc_call_once_eh) is claimed again.
 */
static inline bool __libc_once_claim(unsigned int *state)
{
	unsigned int value = __atomic_load_n(state, __ATOMIC_ACQUIRE);

	while (value != __LIBC_ONCE_DONE) {
		if (value == __LIBC_ONCE_NOT_RUN) {
			if (__atomic_compare_exchange_n(state, &value, __LIBC_ONCE_RUNNING, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
				return true;
			continue;
		}
		if (value == __LIBC_ONCE_RUNNING && !__atomic_compare_exchange_n(state, &value, __LIBC_ONCE_WAITING, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			continue;
		__rtos_futex_wait(state, __LIBC_ONCE_WAITING, TIME_UTC, nullptr);
		value = __atomic_load_n(state, __ATOMIC_ACQUIRE);
	}

	return false;
}

/* Publish DONE (the release pairs with the claim's acquire) or NOT_RUN, and wake any waiters */
static inline void __libc_once_finish(unsigned int *state, unsigned int next)
{
	if (__atomic_exchange_n(state, next, __ATOMIC_RELEASE) == __LIBC_ONCE_WAITING)
		__rtos_futex_wake(state, UINT_MAX);
}

#endif /* _INTERNAL_ONCE_H */
