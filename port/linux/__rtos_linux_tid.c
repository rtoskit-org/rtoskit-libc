/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_linux_tid.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

/* The kernel's thread id, for the priority-inheritance futexes (P10.05); cached per thread */
static thread_local int tid;

int __rtos_linux_tid(void)
{
	if (tid == 0)
		tid = (int)__linux_syscall0(__LINUX_SYS_GETTID);

	return tid;
}
