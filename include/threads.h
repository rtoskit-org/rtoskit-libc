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

#ifndef _THREADS_H
#define _THREADS_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

#include <time.h>

/* once_flag, ONCE_FLAG_INIT and call_once */
#include <__once_flag.h>

/*
 * C23 7.28, built on the thread and futex parts of the RTOS seam
 * (docs/research/threads.md). The objects hold no port data, so the
 * header is the same for every port and core (PLAN D15).
 */

#define TSS_DTOR_ITERATIONS 4

/* A thread is identified by the port's thread record */
struct __rtos_thread;
typedef struct __rtos_thread *thrd_t;

typedef int (*thrd_start_t)(void *);
typedef void (*tss_dtor_t)(void *);

/* A mutex word (0 unlocked, 1 locked, 2 locked with waiters), its type, owner and recursion count */
typedef struct {
	unsigned int __state;
	int __type;
	thrd_t __owner;
	unsigned int __count;
} mtx_t;

/* A sequence number that every signal advances */
typedef struct {
	unsigned int __sequence;
} cnd_t;

/* An index into the key table */
typedef unsigned int tss_t;

enum {
	mtx_plain = 0,
	mtx_recursive = 1,
	mtx_timed = 2,
};

enum {
	thrd_success = 0,
	thrd_busy = 1,
	thrd_error = 2,
	thrd_nomem = 3,
	thrd_timedout = 4,
};

int cnd_broadcast(cnd_t *cond);
void cnd_destroy(cnd_t *cond);
int cnd_init(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_timedwait(cnd_t *__restrict cond, mtx_t *__restrict mtx, const struct timespec *__restrict ts);
int cnd_wait(cnd_t *cond, mtx_t *mtx);

void mtx_destroy(mtx_t *mtx);
int mtx_init(mtx_t *mtx, int type);
int mtx_lock(mtx_t *mtx);
int mtx_timedlock(mtx_t *__restrict mtx, const struct timespec *__restrict ts);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
thrd_t thrd_current(void);
int thrd_detach(thrd_t thr);
int thrd_equal(thrd_t thr0, thrd_t thr1);
[[noreturn]] void thrd_exit(int res);
int thrd_join(thrd_t thr, int *res);
int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void thrd_yield(void);

int tss_create(tss_t *key, tss_dtor_t dtor);
void tss_delete(tss_t key);
void *tss_get(tss_t key);
int tss_set(tss_t key, void *val);

__LIBC_END_DECLS

#endif /* _THREADS_H */
