/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * call_once.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <limits.h>
#include <rtos/rtos-port.h>
#include <threads.h>

/* once_flag states (docs/research/threads.md) */
#define NOT_RUN 0u
#define RUNNING 1u
#define WAITING 2u
#define DONE 3u

/* Owned by the caller running func: done is set once func returns */
struct once_guard {
	unsigned int *state;
	bool done;
};

/*
 * Runs when the runner's frame is left: on return, func finished and the
 * flag moves to DONE; on an exception from func (C++ std::call_once over
 * gthr-c11, issue #121), the flag goes back to NOT_RUN so the next call runs
 * func again. Either way, waiters are woken.
 */
static void once_finish(struct once_guard *guard)
{
	unsigned int next = guard->done ? DONE : NOT_RUN;

	if (__atomic_exchange_n(guard->state, next, __ATOMIC_RELEASE) == WAITING)
		__rtos_futex_wake(guard->state, UINT_MAX);
}

/*
 * C23 7.28.2.1: the caller that moves the flag from NOT_RUN to RUNNING runs
 * func; the others sleep until DONE, which the release store publishes, and
 * claim the flag themselves if it went back to NOT_RUN. This file is built
 * with -fexceptions (threads.mk) so an exception can unwind through it and
 * run once_finish; a longjmp out of func runs no cleanup and leaves the flag
 * RUNNING, as POSIX leaves pthread_once in that case.
 */
void call_once(once_flag *flag, void (*func)(void))
{
	unsigned int state = __atomic_load_n(&flag->__state, __ATOMIC_ACQUIRE);

	while (state != DONE) {
		if (state == NOT_RUN) {
			if (!__atomic_compare_exchange_n(&flag->__state, &state, RUNNING, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
				continue;

			struct once_guard guard [[gnu::cleanup(once_finish)]] = { .state = &flag->__state, .done = false };

			func();
			guard.done = true;
			return;
		}
		if (state == RUNNING && !__atomic_compare_exchange_n(&flag->__state, &state, WAITING, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			continue;
		__rtos_futex_wait(&flag->__state, WAITING, nullptr);
		state = __atomic_load_n(&flag->__state, __ATOMIC_ACQUIRE);
	}
}
