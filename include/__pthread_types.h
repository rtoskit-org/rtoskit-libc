/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __pthread_types.h
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef ___PTHREAD_TYPES_H
#define ___PTHREAD_TYPES_H

/*
 * The POSIX threads types, which both <pthread.h> and <sys/types.h> declare
 * (phase 10, PLAN D148); this piece keeps them defined once. Each layout is
 * the same on every core of a target triple (D15).
 */

/* A thread is the seam's handle, as thrd_t is */
struct __rtos_thread;
typedef struct __rtos_thread *pthread_t;

/* What pthread_create applies: a stack (0 for the port's size, null for the port's own), scheduling and detach state */
typedef struct {
	__SIZE_TYPE__ __stack_size;
	void *__stack;
	int __policy;
	int __priority;
	int __inherit;
	int __detach;
	int __scope;
} pthread_attr_t;

/* The once_flag state machine: 0 not run, 1 running, 2 running with waiters, 3 done (PLAN D152) */
typedef struct {
	unsigned int __state;
} pthread_once_t;

/* An index into the key table */
typedef unsigned int pthread_key_t;

/* A mutex word (0 unlocked, 1 locked, 2 locked with waiters), its type, owner, recursion count and protocol */
typedef struct {
	unsigned int __state;
	int __type;
	struct __rtos_thread *__owner;
	unsigned int __count;
	int __protocol;
} pthread_mutex_t;

typedef struct {
	int __type;
	int __protocol;
	int __pshared;
} pthread_mutexattr_t;

/* A sequence number that every signal advances, and the clock of its timed waits (0 for CLOCK_REALTIME) */
typedef struct {
	unsigned int __sequence;
	int __clock;
} pthread_cond_t;

typedef struct {
	int __clock;
	int __pshared;
} pthread_condattr_t;

#endif /* ___PTHREAD_TYPES_H */
