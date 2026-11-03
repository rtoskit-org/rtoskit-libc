/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * dup.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdio.h>
#include <unistd.h>

/*
 * POSIX.1-2024 dup (issue #113, D144): the seam cannot duplicate a handle, so
 * -1 with ENOSYS, or EBADF for a handle the port rejects. libstdc++ calls it
 * only for a terminal, which isatty never reports.
 */
int dup(int fildes)
{
	long long status = fildes < 0 ? -EBADF : __rtos_seek(fildes, 0, SEEK_CUR);

	errno = status == -EBADF ? EBADF : ENOSYS;
	return -1;
}
