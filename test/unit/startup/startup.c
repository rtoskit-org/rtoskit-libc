/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * startup.c
 *
 * Created on: Oct 24, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <stdio.h>

/*
 * Program start (PLAN D10, D104): the port runs .preinit_array with the
 * initial thread's TLS in place, then __libc_start_main runs .init_array
 * once, then main. Each hook runs exactly once and in that order.
 */

static int order[4];
static int count;
static int preinit_errno;

static void record(int step)
{
	if (count < (int)(sizeof(order) / sizeof(order[0])))
		order[count] = step;
	++count;
}

/* The TLS exists already: errno is the thread's own and starts at zero */
static void preinit_hook(void)
{
	preinit_errno = errno;
	errno = 42;
	record(1);
}

[[gnu::section(".preinit_array"), gnu::used]] static void (*const preinit_entry)(void) = preinit_hook;

[[gnu::constructor]] static void constructor(void)
{
	record(errno == 42 ? 2 : -2);
}

int main(void)
{
	record(3);

	if (count != 3 || order[0] != 1 || order[1] != 2 || order[2] != 3 || preinit_errno != 0) {
		printf("fail: startup order %d: %d %d %d, errno in preinit %d\n", count, order[0], order[1], order[2], preinit_errno);
		return 1;
	}

	puts("pass: startup");
	return 0;
}
