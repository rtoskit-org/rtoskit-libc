/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_create.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

static unsigned int last_lock_token = __LINUX_MAIN_LOCK_TOKEN;

/*
 * The new thread's first function: record itself and take a lock token
 * (even and nonzero, unique for 2^31 threads), then run the entry
 */
static int thread_start(void *argument)
{
	struct __rtos_thread *thread = argument;

	__rtos_linux_self = thread;
	__rtos_lock_token = __atomic_add_fetch(&last_lock_token, 2, __ATOMIC_RELAXED);
	__rtos_thread_exit(thread->entry(thread->arg));
}

/*
 * One mapping per thread: the stack grows down from the thread record, and
 * the TLS block follows the record (docs/research/threads.md). A caller's
 * stack (P10.02) replaces the mapping's: the mapping then holds the record
 * and the TLS block only, and the stack is the caller's to free. The kernel
 * stores the tid before clone returns and clears it when the thread is gone.
 */
int __rtos_thread_create(struct __rtos_thread **result, const struct __rtos_thread_attr *attr, void *(*entry)(void *), void *arg)
{
	struct __rtos_thread *self = __rtos_thread_self();
	size_t stack_size = attr != nullptr && attr->stack_size != 0 ? attr->stack_size : __LINUX_STACK_SIZE;
	size_t own_stack = attr != nullptr && attr->stack != nullptr ? 0 : stack_size;
	size_t tls_size = __libc_tls_block_size(&__rtos_linux_tls_image);
	size_t size = (own_stack + sizeof(struct __rtos_thread) + tls_size + __LINUX_PAGE_SIZE - 1) & ~(__LINUX_PAGE_SIZE - 1);
	int policy = self->policy;
	int priority = self->priority;
	struct __rtos_thread *thread;
	void *stack_top;
	long mapping;
	long status;
	void *tp;

	if (attr != nullptr && !attr->inherit) {
		int min;
		int max;

		status = __rtos_sched_priority_range(attr->policy, &min, &max);
		if (status < 0)
			return (int)status;
		if (attr->priority < min || attr->priority > max)
			return -EINVAL;
		policy = attr->policy;
		priority = attr->priority;
	}

	__rtos_linux_reap();

	mapping = __linux_syscall6(__LINUX_SYS_MMAP, 0, (long)size, __LINUX_PROT_READ | __LINUX_PROT_WRITE, __LINUX_MAP_PRIVATE | __LINUX_MAP_ANONYMOUS, -1, 0);
	if (__linux_is_error(mapping))
		return -ENOMEM;

	thread = (struct __rtos_thread *)(mapping + own_stack);
	*thread = (struct __rtos_thread){ .tid = 0, .state = __LINUX_THREAD_JOINABLE, .entry = entry, .arg = arg, .mapping = (void *)mapping, .mapping_size = size, .policy = policy, .priority = priority };
	tp = __libc_tls_init(thread + 1, &__rtos_linux_tls_image);
	/* The stack's top, 16-byte aligned as both ABIs want it */
	stack_top = own_stack != 0 ? (void *)thread : (void *)(((uintptr_t)attr->stack + stack_size) & ~(uintptr_t)15);

	status = __rtos_linux_clone(__LINUX_CLONE_THREAD_FLAGS, stack_top, &thread->tid, &thread->tid, tp, thread_start, thread);
	if (__linux_is_error(status)) {
		__linux_syscall2(__LINUX_SYS_MUNMAP, mapping, (long)size);
		return (int)status;
	}

	*result = thread;
	return 0;
}
