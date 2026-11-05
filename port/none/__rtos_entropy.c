/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_entropy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

/*
 * Bare metal has no entropy source of its own: a board with a hardware
 * random number generator replaces this weak default (issue #127)
 */
[[gnu::weak]] int __rtos_entropy(void *, size_t)
{
	return -ENOSYS;
}
