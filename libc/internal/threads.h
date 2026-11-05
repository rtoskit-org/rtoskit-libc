/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * threads.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_THREADS_H
#define _INTERNAL_THREADS_H

#include <threads.h>

/*
 * <threads.h> on the seam's threads and futexes (docs/research/threads.md).
 */

/* Mutex word states */
#define __LIBC_MUTEX_UNLOCKED 0u
#define __LIBC_MUTEX_LOCKED 1u
#define __LIBC_MUTEX_CONTENDED 2u

/* Lock with an optional TIME_UTC deadline: thrd_success, thrd_timedout or thrd_error */
int __libc_mutex_lock(mtx_t *mtx, const struct timespec *deadline);
int __libc_mutex_unlock(mtx_t *mtx);

/* cnd_wait and cnd_timedwait */
int __libc_cond_wait(cnd_t *cond, mtx_t *mtx, const struct timespec *deadline);

/* Live threads; the program ends when the last one calls thrd_exit or pthread_exit (C23 7.28.5.5p5) */
extern unsigned int __libc_thread_count;

/* The end of a thread for thrd_exit and pthread_exit: destructors, then the seam's exit or exit() (P10.03) */
[[noreturn]] void __libc_thread_exit(void *result);

/* What pthread_create hands the new thread */
struct __libc_pthread_start {
	void *(*func)(void *);
	void *arg;
};

void *__libc_pthread_start(void *start);

/* What thrd_create hands the new thread */
struct __libc_thread_start {
	thrd_start_t func;
	void *arg;
};

void *__libc_thread_start(void *start);

/*
 * Thread-specific storage: a key is in use while its generation is odd; a
 * thread's value counts only if it was set under the key's current
 * generation, so a new key reads as null everywhere (7.28.6.1p3).
 */
#define __LIBC_TSS_MAX 32

struct __libc_tss_key {
	unsigned int generation;
	tss_dtor_t dtor;
};

struct __libc_tss_value {
	void *value;
	unsigned int generation;
};

extern struct __libc_tss_key __libc_tss_keys[__LIBC_TSS_MAX];
extern thread_local struct __libc_tss_value __libc_tss_values[__LIBC_TSS_MAX];

/* Run by thrd_exit when tss_create is linked (a weak reference, PLAN D43) */
[[gnu::weak]] void __libc_tss_run_destructors(void);

#endif /* _INTERNAL_THREADS_H */
