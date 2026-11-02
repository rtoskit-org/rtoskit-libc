/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * close.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <unistd.h>

/* POSIX.1-2024 close, over the seam (PLAN D130) */
int close(int fildes)
{
	int status = __rtos_close(fildes);

	if (status < 0) {
		errno = -status;
		return -1;
	}

	return 0;
}
