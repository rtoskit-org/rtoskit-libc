/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_thread_exit.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>
#include <stdlib.h>

#include "internal/cxa.h"
#include "internal/threads.h"

/* The initial thread is live from the start */
unsigned int __libc_thread_count = 1;

/*
 * The end of a thread, for thrd_exit and pthread_exit (P10.03): the C++
 * thread_local destructors, then the key destructors when linked (PLAN
 * D121, D43); the last thread to end ends the program as exit(EXIT_SUCCESS)
 * would (C23 7.28.5.5p5, POSIX pthread_exit).
 */
[[noreturn]] void __libc_thread_exit(void *result)
{
	if (__libc_cxa_thread_run != nullptr)
		__libc_cxa_thread_run();

	if (__libc_tss_run_destructors != nullptr)
		__libc_tss_run_destructors();

	if (__atomic_sub_fetch(&__libc_thread_count, 1, __ATOMIC_ACQ_REL) == 0)
		exit(EXIT_SUCCESS);

	__rtos_thread_exit(result);
}
