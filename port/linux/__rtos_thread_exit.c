/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_exit.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * A detached thread cannot unmap the stack it runs on, so it joins the
 * zombie list, which a later create or join clears once the kernel has
 * cleared its tid. Exiting the initial thread leaves the others running.
 */
[[noreturn]] void __rtos_thread_exit(void *result)
{
	struct __rtos_thread *thread = __rtos_thread_self();

	thread->result = result;
	if (__atomic_exchange_n(&thread->state, __LINUX_THREAD_EXITED, __ATOMIC_ACQ_REL) == __LINUX_THREAD_DETACHED && thread != &__rtos_linux_main_thread) {
		thread->next = __atomic_load_n(&__rtos_linux_zombies, __ATOMIC_RELAXED);
		while (!__atomic_compare_exchange_n(&__rtos_linux_zombies, &thread->next, thread, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED))
			continue;
	}

	while (true)
		__linux_syscall1(__LINUX_SYS_EXIT, 0);
}
