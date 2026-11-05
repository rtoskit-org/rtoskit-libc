/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sched.h
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SCHED_H
#define _SCHED_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * POSIX.1-2024 <sched.h>, the part of it the POSIX subset needs (PLAN D128):
 * the scheduling policies and parameter a port's own <pthread.h> uses, and
 * sched_yield over the seam's __rtos_thread_yield. What a policy means is the
 * RTOS's to decide.
 */
struct sched_param {
	int sched_priority;
};

#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2

int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_yield(void);

__LIBC_END_DECLS

#endif /* _SCHED_H */
