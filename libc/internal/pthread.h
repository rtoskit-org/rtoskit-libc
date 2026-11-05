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

#ifndef _INTERNAL_PTHREAD_H
#define _INTERNAL_PTHREAD_H

#include <pthread.h>

/*
 * <pthread.h>'s mutexes and condition variables over the seam's futexes
 * (P10.04, PLAN D153): the three-state mutex word of <threads.h> (internal/
 * threads.h's __LIBC_MUTEX_*), with the type deciding what the owner's own
 * relock and a stranger's unlock do. Each returns 0 or an errno.
 */

/* Lock, waiting until the deadline (null: none) on the time base (TIME_UTC, TIME_MONOTONIC) */
int __libc_pthread_mutex_lock(pthread_mutex_t *mutex, int base, const struct timespec *deadline);
int __libc_pthread_mutex_unlock(pthread_mutex_t *mutex);

/*
 * Read-write locks (P10.06, PLAN D155): __state counts the readers, or is
 * __LIBC_RWLOCK_WRITER while a writer holds it; every release advances
 * __sequence and wakes the waiters, who read the sequence before the state
 * so that no release is missed
 */
#define __LIBC_RWLOCK_WRITER 0xffffffffu

int __libc_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock, int base, const struct timespec *deadline);
int __libc_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock, int base, const struct timespec *deadline);

/* Release mutex, wait for a signal or the deadline, and take mutex back */
int __libc_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, int base, const struct timespec *deadline);

/*
 * The seam time base of a POSIX clock, or -1 for a clock a timed wait cannot
 * use: CLOCK_REALTIME is TIME_UTC and CLOCK_MONOTONIC is TIME_MONOTONIC
 * (PLAN D132); 0 is a condition variable's default, CLOCK_REALTIME
 */
static inline int __libc_pthread_clock_base(int clock)
{
	return clock == 0 || clock == TIME_UTC ? TIME_UTC : clock == TIME_MONOTONIC ? TIME_MONOTONIC :
																				  -1;
}

#endif /* _INTERNAL_PTHREAD_H */
