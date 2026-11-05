/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_join.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * Wait until the kernel clears the tid (a shared futex wake, the kind
 * CLONE_CHILD_CLEARTID sends), then the stack is free to unmap.
 */
int __rtos_thread_join(struct __rtos_thread *thread, void **result)
{
	int tid;

	if (thread == &__rtos_linux_main_thread || thread == __rtos_thread_self() || __atomic_load_n(&thread->state, __ATOMIC_ACQUIRE) == __LINUX_THREAD_DETACHED)
		return -EINVAL;

	while ((tid = __atomic_load_n(&thread->tid, __ATOMIC_ACQUIRE)) != 0)
		__linux_syscall6(__LINUX_SYS_FUTEX, (long)&thread->tid, __LINUX_FUTEX_WAIT, tid, 0, 0, 0);

	if (result != nullptr)
		*result = thread->result;
	__linux_syscall2(__LINUX_SYS_MUNMAP, (long)thread->mapping, (long)thread->mapping_size);
	__rtos_linux_reap();

	return 0;
}
