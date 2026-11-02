/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * lseek.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <unistd.h>

/* POSIX.1-2024 lseek: the seam's whence values are SEEK_SET, SEEK_CUR and SEEK_END (PLAN D129) */
off_t lseek(int fildes, off_t offset, int whence)
{
	long long position = __rtos_seek(fildes, offset, whence);

	if (position < 0) {
		errno = (int)-position;
		return -1;
	}

	return position;
}
