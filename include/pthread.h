/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread.h
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <__cdefs.h>
#include <__pthread_types.h>
#include <sched.h>
#include <sys/types.h>
#include <time.h>

__LIBC_BEGIN_DECLS

/*
 * POSIX.1-2024 <pthread.h> over the RTOS seam (phase 10, PLAN D148-D152,
 * docs/research/pthreads.md). Cancellation, signals, pthread_atfork,
 * process-shared objects and robust mutexes are out of scope (D148, D151).
 */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1

/* Every thread competes with every other: PTHREAD_SCOPE_PROCESS is not supported */
#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1

/* Objects are private to the program: PTHREAD_PROCESS_SHARED is not supported */
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

/* NORMAL deadlocks on its owner's relock; DEFAULT is NORMAL */
#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL

/* Priority inheritance where the port supports it (P10.05); priority protection is not supported */
#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

/* A default mutex, and a condition variable on CLOCK_REALTIME */
#define PTHREAD_MUTEX_INITIALIZER \
	{                             \
		0, 0, 0, 0, 0             \
	}
#define PTHREAD_COND_INITIALIZER \
	{                            \
		0, 0                     \
	}

#define PTHREAD_ONCE_INIT \
	{                     \
		0                 \
	}

int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_getinheritsched(const pthread_attr_t *__restrict attr, int *__restrict inheritsched);
int pthread_attr_getschedparam(const pthread_attr_t *__restrict attr, struct sched_param *__restrict param);
int pthread_attr_getschedpolicy(const pthread_attr_t *__restrict attr, int *__restrict policy);
int pthread_attr_getscope(const pthread_attr_t *__restrict attr, int *__restrict scope);
int pthread_attr_getstack(const pthread_attr_t *__restrict attr, void **__restrict stackaddr, __SIZE_TYPE__ *__restrict stacksize);
int pthread_attr_getstacksize(const pthread_attr_t *__restrict attr, __SIZE_TYPE__ *__restrict stacksize);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_setschedparam(pthread_attr_t *__restrict attr, const struct sched_param *__restrict param);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, __SIZE_TYPE__ stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, __SIZE_TYPE__ stacksize);

int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_clockwait(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex, clockid_t clock_id, const struct timespec *__restrict abstime);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *__restrict cond, const pthread_condattr_t *__restrict attr);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_timedwait(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex, const struct timespec *__restrict abstime);
int pthread_cond_wait(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_getclock(const pthread_condattr_t *__restrict attr, clockid_t *__restrict clock_id);
int pthread_condattr_getpshared(const pthread_condattr_t *__restrict attr, int *__restrict pshared);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

int pthread_create(pthread_t *__restrict thread, const pthread_attr_t *__restrict attr, void *(*start_routine)(void *), void *__restrict arg);
int pthread_detach(pthread_t thread);
int pthread_equal(pthread_t t1, pthread_t t2);
__attribute__((__noreturn__)) void pthread_exit(void *value_ptr);
int pthread_getschedparam(pthread_t thread, int *__restrict policy, struct sched_param *__restrict param);
int pthread_join(pthread_t thread, void **value_ptr);

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

int pthread_mutex_clocklock(pthread_mutex_t *__restrict mutex, clockid_t clock_id, const struct timespec *__restrict abstime);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_init(pthread_mutex_t *__restrict mutex, const pthread_mutexattr_t *__restrict attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *__restrict mutex, const struct timespec *__restrict abstime);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *__restrict attr, int *__restrict protocol);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *__restrict attr, int *__restrict pshared);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *__restrict attr, int *__restrict type);
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
pthread_t pthread_self(void);
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_setschedprio(pthread_t thread, int prio);

__LIBC_END_DECLS

#endif /* _PTHREAD_H */
