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

/* One thread (P10.05): the word is free or the one thread holds it */
int __rtos_mutex_trylock_pi(unsigned int *word)
{
	if (*word != 0)
		return -EBUSY;

	*word = 1;
	return 0;
}
