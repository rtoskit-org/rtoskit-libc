/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * tmpfile.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdio.h>

#include "internal/file.h"

/* The next candidate for tmpfile, under __rtos_stdio_lock */
static unsigned int next_file;

/*
 * C23 7.23.4.3: a new file opened "wb+" that is removed when closed: created
 * exclusively under one of the port's candidate names with the TEMPORARY flag.
 */
FILE *tmpfile(void)
{
	char name[L_tmpnam];

	for (unsigned int attempt = 0; attempt < TMP_MAX; ++attempt) {
		unsigned int index;

		__rtos_lock_acquire(&__rtos_stdio_lock);
		/* Spread from tmpnam's sequence so the two rarely collide */
		index = next_file++ + TMP_MAX;
		__rtos_lock_release(&__rtos_stdio_lock);

		int status = __rtos_tmpname(name, sizeof(name), index);

		if (status < 0) {
			errno = -status;
			return nullptr;
		}

		int handle = __rtos_open(name, __RTOS_OPEN_READ | __RTOS_OPEN_WRITE | __RTOS_OPEN_CREATE | __RTOS_OPEN_EXCLUSIVE | __RTOS_OPEN_TEMPORARY);

		if (handle >= 0) {
			FILE *stream = __libc_file_new(handle, __LIBC_FILE_READ | __LIBC_FILE_WRITE);

			/* Closing the handle removes the temporary file */
			if (stream == nullptr)
				__rtos_close(handle);
			return stream;
		}
		if (handle != -EEXIST) {
			errno = -handle;
			return nullptr;
		}
	}

	errno = EEXIST;
	return nullptr;
}
