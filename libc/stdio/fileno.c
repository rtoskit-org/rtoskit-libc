/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fileno.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <stdio.h>

#include "internal/file.h"

/* POSIX.1-2024 fileno: a stream's descriptor is its seam handle (PLAN D129) */
int fileno(FILE *stream)
{
	int handle;

	__libc_file_lock(stream);
	handle = stream->handle;
	__libc_file_unlock(stream);

	return handle;
}
