/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fstat.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/*
 * POSIX.1-2024 fstat over the seam (PLAN D134): the seam knows only a
 * handle's offset. A handle that seeks is a regular file whose size is its
 * end, found by seeking there and back; one that cannot (ESPIPE: a console,
 * a pipe, every handle of port/none) is reported as a character device of
 * size 0. The other fields are zero. Another thread using the same handle
 * between the two seeks would see the end offset, as with any shared
 * descriptor offset.
 */
int fstat(int fildes, struct stat *buf)
{
	long long current = fildes < 0 ? -EBADF : __rtos_seek(fildes, 0, SEEK_CUR);
	long long end = 0;

	if (current < 0 && current != -ESPIPE) {
		errno = (int)-current;
		return -1;
	}

	if (current >= 0) {
		end = __rtos_seek(fildes, 0, SEEK_END);
		if (end < 0) {
			errno = (int)-end;
			return -1;
		}
		__rtos_seek(fildes, current, SEEK_SET);
	}

	memset(buf, 0, sizeof(*buf));
	buf->st_mode = current >= 0 ? S_IFREG : S_IFCHR;
	buf->st_size = end;
	buf->st_nlink = 1;

	return 0;
}
