/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * thrd_exit.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>
#include <threads.h>

#include "internal/threads.h"

/*
 * C23 7.28.5.5: the thread ends with res, which thrd_join returns, after the
 * C++ thread_local and the tss destructors; the last thread to end ends the
 * program as exit(EXIT_SUCCESS) would (__libc_thread_exit, P10.03)
 */
[[noreturn]] void thrd_exit(int res)
{
	__libc_thread_exit((void *)(intptr_t)res);
}
