/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_mutex_lock_pi.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <time.h>

/*
 * One thread (P10.05): no other thread can hold the word, so taking it is
 * all there is, and the one thread taking it again is EDEADLK. With no one
 * to boost, inheritance holds trivially.
 */
int __rtos_mutex_lock_pi(unsigned int *word, int base, const struct timespec *deadline)
{
	if (base != TIME_UTC && base != TIME_MONOTONIC)
		return -EINVAL;
	if (deadline != nullptr && (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000))
		return -EINVAL;
	if (*word != 0)
		return -EDEADLK;

	*word = 1;
	return 0;
}
