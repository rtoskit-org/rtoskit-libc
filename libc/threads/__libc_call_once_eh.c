/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_call_once_eh.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <threads.h>

#include "internal/once.h"

/* Owned by the caller running func: done is set once func returns */
struct once_guard {
	unsigned int *state;
	bool done;
};

/* Runs as the runner's frame is left, by return or by an exception */
static void once_guard_finish(struct once_guard *guard)
{
	__libc_once_finish(guard->state, guard->done ? __LIBC_ONCE_DONE : __LIBC_ONCE_NOT_RUN);
}

/*
 * call_once for C++ (issue #121, PLAN D145): the toolchain's gthr-c11.h
 * __gthread_once calls this. It is built with -fexceptions (threads.mk), so
 * an exception from func unwinds through it and the cleanup puts the flag
 * back to NOT_RUN and wakes the waiters, one of which then runs func, as
 * std::call_once requires ([thread.once.callonce]). A longjmp out of func
 * runs no cleanup and leaves the flag RUNNING.
 */
void __libc_call_once_eh(once_flag *flag, void (*func)(void))
{
	if (!__libc_once_claim(&flag->__state))
		return;

	struct once_guard guard [[gnu::cleanup(once_guard_finish)]] = { .state = &flag->__state, .done = false };

	func();
	guard.done = true;
}
