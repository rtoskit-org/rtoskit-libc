/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stat.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <sys/stat.h>

/*
 * POSIX.1-2024 stat over the seam (issue #113, D144): the path opened for
 * reading, fstat's answer for the handle (D134), the handle closed. A port
 * that cannot open the path (a directory, port/none) reports why.
 */
int stat(const char *restrict path, struct stat *restrict buf)
{
	int handle = __rtos_open(path, __RTOS_OPEN_READ);
	int status;

	if (handle < 0) {
		errno = -handle;
		return -1;
	}

	status = fstat(handle, buf);
	__rtos_close(handle);
	return status;
}
