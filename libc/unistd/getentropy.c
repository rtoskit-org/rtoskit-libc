/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * getentropy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <limits.h>
#include <rtos/rtos-port.h>
#include <unistd.h>

/*
 * POSIX.1-2024 getentropy: length bytes from the port's entropy source, or
 * -1 with EINVAL when length is over GETENTROPY_MAX, and with the port's
 * ENOSYS (no source) or EIO otherwise. A board without a source keeps
 * failing; nothing here falls back to predictable bytes (issue #127).
 */
int getentropy(void *buffer, size_t length)
{
	int status;

	if (length > GETENTROPY_MAX) {
		errno = EINVAL;
		return -1;
	}

	status = __rtos_entropy(buffer, length);
	if (status < 0) {
		errno = -status;
		return -1;
	}

	return 0;
}
