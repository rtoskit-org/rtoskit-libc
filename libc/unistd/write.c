/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * write.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <unistd.h>

/* POSIX.1-2024 write, over the seam (PLAN D130): the byte count, or -1 with errno */
ssize_t write(int fildes, const void *buf, size_t nbyte)
{
	ptrdiff_t count = __rtos_write(fildes, buf, nbyte);

	if (count < 0) {
		errno = (int)-count;
		return -1;
	}

	return count;
}
