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

#include <pthread.h>
#include <stdint.h>
#include <threads.h>

/*
 * C23 7.28.5.5 over pthread_exit (P10.07): the thread ends with res, which
 * thrd_join returns, after the C++ thread_local and the key destructors; the
 * last thread to end ends the program as exit(EXIT_SUCCESS) would
 */
[[noreturn]] void thrd_exit(int res)
{
	pthread_exit((void *)(intptr_t)res);
}
