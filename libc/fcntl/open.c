/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * open.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <fcntl.h>
#include <rtos/rtos-port.h>

/*
 * POSIX.1-2024 open, over the seam (PLAN D130): the descriptor is the seam
 * handle (D129). The seam has no file modes, so the mode argument O_CREAT
 * takes is not read. O_NONBLOCK has no seam flag and no effect on a file the
 * seam opens; other bits are ignored, as Linux does.
 */
int open(const char *path, int oflag, ...)
{
	unsigned int flags;

	switch (oflag & O_ACCMODE) {
		case O_RDONLY:
			flags = __RTOS_OPEN_READ;
			break;
		case O_WRONLY:
			flags = __RTOS_OPEN_WRITE;
			break;
		case O_RDWR:
			flags = __RTOS_OPEN_READ | __RTOS_OPEN_WRITE;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	if (oflag & O_APPEND)
		flags |= __RTOS_OPEN_APPEND;
	if (oflag & O_CREAT)
		flags |= __RTOS_OPEN_CREATE;
	if (oflag & O_TRUNC)
		flags |= __RTOS_OPEN_TRUNCATE;
	if (oflag & O_EXCL)
		flags |= __RTOS_OPEN_EXCLUSIVE;

	int handle = __rtos_open(path, flags);

	if (handle < 0) {
		errno = -handle;
		return -1;
	}

	return handle;
}
