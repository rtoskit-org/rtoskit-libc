/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fopen.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdio.h>

#include "internal/file.h"

/* C23 7.23.5.3 */
FILE *fopen(const char *restrict filename, const char *restrict mode)
{
	unsigned int open_flags;
	unsigned short stream_flags;

	if (!__libc_file_mode(mode, &open_flags, &stream_flags))
		return nullptr;

	int handle = __rtos_open(filename, open_flags);

	if (handle < 0) {
		errno = -handle;
		return nullptr;
	}

	FILE *stream = __libc_file_new(handle, stream_flags);

	if (stream == nullptr)
		__rtos_close(handle);

	return stream;
}
