/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_thread_exit.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>
#include <stdlib.h>

#include "rtos-none.h"

/* libc ends the program itself when its last thread exits; this is never reached with one thread */
[[noreturn]] void __rtos_thread_exit(void *)
{
	__rtos_exit(EXIT_SUCCESS);
}
