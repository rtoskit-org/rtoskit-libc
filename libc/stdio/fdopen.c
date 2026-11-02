/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fdopen.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <stdio.h>

#include "internal/file.h"

/*
 * POSIX.1-2024 fdopen: a stream for an open seam handle (PLAN D129). The mode
 * is an fopen mode; "w" does not truncate and "x" has no effect, since the
 * file is already open. The library cannot ask the port for a handle's access
 * mode, so a mode the handle does not allow fails at the first read or write.
 * On failure the descriptor stays open, for the caller to close.
 */
FILE *fdopen(int fildes, const char *mode)
{
	unsigned int open_flags;
	unsigned short stream_flags;

	if (fildes < 0) {
		errno = EBADF;
		return nullptr;
	}

	if (!__libc_file_mode(mode, &open_flags, &stream_flags))
		return nullptr;

	return __libc_file_new(fildes, stream_flags);
}
