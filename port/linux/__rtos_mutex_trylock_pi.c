/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_mutex_trylock_pi.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/* Only a free word can be taken without waiting (P10.05) */
int __rtos_mutex_trylock_pi(unsigned int *word)
{
	unsigned int expected = 0;

	return __atomic_compare_exchange_n(word, &expected, (unsigned int)__rtos_linux_tid(), false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED) ? 0 : -EBUSY;
}
