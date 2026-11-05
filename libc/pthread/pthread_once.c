/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_once.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

#include "internal/once.h"

/*
 * POSIX pthread_once on call_once's state machine (PLAN D145, D152). Built
 * without unwind tables, so a C program pays nothing for C++'s exception
 * rules; pthread_once_t has once_flag's layout, so the toolchain's
 * __gthread_once (std::call_once) calls __libc_call_once_eh on it instead.
 */
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
	if (!__libc_once_claim(&once_control->__state))
		return 0;

	init_routine();
	__libc_once_finish(&once_control->__state, __LIBC_ONCE_DONE);

	return 0;
}
