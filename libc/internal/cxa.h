/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cxa.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_CXA_H
#define _INTERNAL_CXA_H

/*
 * The C++ ABI's termination hooks (Itanium C++ ABI 3.3.5 and 3.4, which the
 * Arm C++ ABI adopts; PLAN D121, docs/research/cxx-runtime.md). The compiler
 * and the C++ runtime call these by name; libstdc++'s <cxxabi.h> declares
 * them for C++.
 */

/* A destructor registered with __cxa_atexit, and the atexit slot it runs from */
struct __libc_cxa_entry {
	void (*func)(void *);
	void *arg;
	void *dso;
	unsigned int level;
};

/*
 * Entries live in blocks: the first is static, the rest come from malloc when
 * a program registers more than one block's worth (C++ registers one per
 * static object with a destructor). The newest block is searched first.
 */
#define __LIBC_CXA_BLOCK_ENTRIES 32

struct __libc_cxa_block {
	struct __libc_cxa_block *older;
	unsigned int count;
	struct __libc_cxa_entry entries[__LIBC_CXA_BLOCK_ENTRIES];
};

/* Protected by __rtos_atexit_lock */
extern struct __libc_cxa_block *__libc_cxa_newest;

/* A destructor of a thread_local object, run when its thread ends */
struct __libc_cxa_thread_entry {
	struct __libc_cxa_thread_entry *next;
	void (*func)(void *);
	void *obj;
};

/* This thread's list, newest first */
extern thread_local struct __libc_cxa_thread_entry *__libc_cxa_thread_entries;

int __cxa_atexit(void (*func)(void *), void *arg, void *dso);
void __cxa_finalize(void *dso);
int __cxa_thread_atexit(void (*func)(void *), void *obj, void *dso);
extern void *__dso_handle;

/*
 * Runs and frees this thread's thread_local destructors: from thrd_exit, and
 * for the thread that calls exit from the atexit runner, before any static
 * destructor (C++ [basic.start.term]p1). Linked only with __cxa_thread_atexit
 * (a weak reference).
 */
[[gnu::weak]] void __libc_cxa_thread_run(void);

#endif /* _INTERNAL_CXA_H */
