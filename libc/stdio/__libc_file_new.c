/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_file_new.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdio.h>
#include <stdlib.h>

#include "internal/file.h"

/*
 * C23 7.24.4.4p4: exit closes the streams fopen and tmpfile made, which also
 * removes tmpfile files. Only this object makes them, so closing at exit is
 * its destructor, and a program that opens no file links no fclose (PLAN D52).
 */
[[gnu::destructor]] static void close_at_exit(void)
{
	while (__libc_files != nullptr)
		fclose(__libc_files);
}

/*
 * An opened file is fully buffered: it is not taken to be an interactive device
 * (C23 7.23.5.3p8). On failure the handle stays open: fopen and tmpfile close
 * it, while fdopen leaves it to its caller (POSIX.1-2024).
 */
FILE *__libc_file_new(int handle, unsigned short flags)
{
	FILE *stream = calloc(1, sizeof(*stream));
	int status;

	if (stream == nullptr)
		return nullptr;

	status = __rtos_lock_create(&stream->lock);
	if (status < 0) {
		errno = -status;
		free(stream);
		return nullptr;
	}

	stream->handle = handle;
	stream->flags = flags | __LIBC_FILE_ALLOCATED;
	__libc_file_link(stream);

	return stream;
}
