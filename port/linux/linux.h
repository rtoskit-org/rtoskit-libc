/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * linux.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LINUX_H
#define _LINUX_H

#include <time.h>

#include "linux-arch.h"

#define __LINUX_PT_TLS 7

#define __LINUX_PROT_READ 0x1
#define __LINUX_PROT_WRITE 0x2
#define __LINUX_MAP_PRIVATE 0x02
#define __LINUX_MAP_ANONYMOUS 0x20
#define __LINUX_MAP_NORESERVE 0x4000

#define __LINUX_SIGABRT 6

/* openat flags and arguments; the values are the same on every architecture except O_LARGEFILE */
#define __LINUX_O_RDONLY 0
#define __LINUX_O_WRONLY 1
#define __LINUX_O_RDWR 2
#define __LINUX_O_CREAT 0x40
#define __LINUX_O_EXCL 0x80
#define __LINUX_O_TRUNC 0x200
#define __LINUX_O_APPEND 0x400
#define __LINUX_O_CLOEXEC 0x80000
#define __LINUX_AT_FDCWD -100
#define __LINUX_CREATE_MODE 0666

/* Clock IDs for the time bases, and the kernel's 64-bit timespec */
#define __LINUX_CLOCK_REALTIME 0
#define __LINUX_CLOCK_MONOTONIC 1
#define __LINUX_CLOCK_PROCESS_CPUTIME_ID 2
#define __LINUX_CLOCK_THREAD_CPUTIME_ID 3

struct __linux_timespec {
	long long tv_sec;
	long long tv_nsec;
};

/* The kernel clock for a TIME_* base, or -1 */
static inline int __linux_clock(int base)
{
	switch (base) {
		case TIME_UTC:
			return __LINUX_CLOCK_REALTIME;
		case TIME_MONOTONIC:
			return __LINUX_CLOCK_MONOTONIC;
		case TIME_ACTIVE:
			return __LINUX_CLOCK_PROCESS_CPUTIME_ID;
		case TIME_THREAD_ACTIVE:
			return __LINUX_CLOCK_THREAD_CPUTIME_ID;
		default:
			return -1;
	}
}

/* The heap: reserved address space, committed by the kernel page by page as it is touched */
#define __LINUX_HEAP_SIZE (64UL << 20)

/* Locks the port can create at run time */
#define __LINUX_LOCK_POOL_SIZE 32

/*
 * A recursive futex mutex on the seam's lock word (rtos-port.h, PLAN D79):
 * the word holds the owner's token and the slow bit, and depth counts the
 * owner's acquisitions beyond the first.
 */
struct __rtos_lock {
	unsigned int word;
	unsigned int depth;
	unsigned int allocated;
};

/* The initial thread's lock token; the others count up from it in steps of two */
#define __LINUX_MAIN_LOCK_TOKEN 2u

/* futex operations */
#define __LINUX_FUTEX_WAIT 0
#define __LINUX_FUTEX_WAKE 1
#define __LINUX_FUTEX_WAIT_BITSET 9
#define __LINUX_FUTEX_PRIVATE 128
#define __LINUX_FUTEX_CLOCK_REALTIME 256
#define __LINUX_FUTEX_BITSET_ANY 0xffffffff

/* clone flags for a thread sharing everything, with its own TLS and a tid the kernel clears at exit */
#define __LINUX_CLONE_VM 0x00000100
#define __LINUX_CLONE_FS 0x00000200
#define __LINUX_CLONE_FILES 0x00000400
#define __LINUX_CLONE_SIGHAND 0x00000800
#define __LINUX_CLONE_THREAD 0x00010000
#define __LINUX_CLONE_SYSVSEM 0x00040000
#define __LINUX_CLONE_SETTLS 0x00080000
#define __LINUX_CLONE_PARENT_SETTID 0x00100000
#define __LINUX_CLONE_CHILD_CLEARTID 0x00200000
#define __LINUX_CLONE_THREAD_FLAGS (__LINUX_CLONE_VM | __LINUX_CLONE_FS | __LINUX_CLONE_FILES | __LINUX_CLONE_SIGHAND | __LINUX_CLONE_THREAD | __LINUX_CLONE_SYSVSEM | __LINUX_CLONE_SETTLS | __LINUX_CLONE_PARENT_SETTID | __LINUX_CLONE_CHILD_CLEARTID)

/* Each thread's mapping: its stack (unless the caller gave one), then its record, then its TLS block */
#define __LINUX_STACK_SIZE (256UL << 10)
#define __LINUX_PAGE_SIZE 4096UL

#define __LINUX_THREAD_JOINABLE 0u
#define __LINUX_THREAD_DETACHED 1u
#define __LINUX_THREAD_EXITED 2u

/* Linux's own priority ranges (sched_get_priority_min/max): SCHED_FIFO and SCHED_RR 1-99, SCHED_OTHER 0 */
#define __LINUX_RT_PRIORITY_MIN 1
#define __LINUX_RT_PRIORITY_MAX 99

struct __rtos_thread {
	int tid; /* set by clone, cleared and futex-woken by the kernel when the thread is gone */
	void *result;
	unsigned int state;
	void *(*entry)(void *);
	void *arg;
	int policy; /* recorded, not applied: the test port runs unprivileged (P10.02) */
	int priority;
	void *mapping;
	size_t mapping_size;
	struct __rtos_thread *next; /* on the list of detached threads waiting to be unmapped */
};

/* The initial thread, and the calling thread's record (null in the initial thread) */
extern struct __rtos_thread __rtos_linux_main_thread;
extern thread_local struct __rtos_thread *__rtos_linux_self;

/* Detached threads that have exited; __rtos_linux_reap unmaps those the kernel has finished with */
extern struct __rtos_thread *__rtos_linux_zombies;
void __rtos_linux_reap(void);

/* The spin lock of the atomics critical section */
extern unsigned int __rtos_linux_atomic_lock;

/*
 * clone(2) in assembly: the child starts on stack and runs fn(arg), then
 * exits; the parent gets the child's tid or a negated errno.
 */
long __rtos_linux_clone(unsigned long flags, void *stack, int *parent_tid, int *child_tid, void *tls, int (*fn)(void *), void *arg);

/* Kernel results in this range are negated error numbers */
static inline bool __linux_is_error(long result)
{
	return result < 0 && result > -4096;
}

#include "rtos-linux.h"

/* Defined by the linker when the ELF header is part of a loaded segment */
extern const struct __linux_ehdr __ehdr_start;

/* The program's entry point, called through __libc_start_main */
extern int main(int argc, char **argv);

[[noreturn]] void __rtos_linux_start(long *stack);

#endif /* _LINUX_H */
