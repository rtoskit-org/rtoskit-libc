/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread_exit.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <pthread.h>

#include "internal/threads.h"

/* POSIX pthread_exit: the thread ends with value_ptr, which pthread_join returns */
[[noreturn]] void pthread_exit(void *value_ptr)
{
	__libc_thread_exit(value_ptr);
}
