/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fcntl.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <fcntl.h>
#include <rtos/rtos-port.h>
#include <stdio.h>

/*
 * POSIX.1-2024 fcntl, the descriptor flags only (PLAN D134). Nothing here
 * executes another program, so FD_CLOEXEC has no effect: F_SETFD accepts
 * it and F_GETFD reports no flags. The seam cannot validate a handle except
 * by using it, so the descriptor is checked with a zero-length seek, which
 * fails with EBADF only for a handle the port does not know.
 */
int fcntl(int fildes, int cmd, ...)
{
	if (cmd != F_GETFD && cmd != F_SETFD) {
		errno = EINVAL;
		return -1;
	}

	long long status = fildes < 0 ? -EBADF : __rtos_seek(fildes, 0, SEEK_CUR);

	if (status == -EBADF) {
		errno = EBADF;
		return -1;
	}

	return 0;
}
