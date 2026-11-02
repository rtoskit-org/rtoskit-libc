/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * reallocarray.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <stdckdint.h>
#include <stdlib.h>

/* POSIX.1-2024 reallocarray: realloc of nelem * elsize, failing with ENOMEM when the product wraps around */
void *reallocarray(void *ptr, size_t nelem, size_t elsize)
{
	size_t total;

	if (ckd_mul(&total, nelem, elsize)) {
		errno = ENOMEM;
		return nullptr;
	}

	return realloc(ptr, total);
}
