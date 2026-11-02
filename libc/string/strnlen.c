/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strnlen.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <string.h>

/* POSIX.1-2024 strnlen: never reads past maxlen bytes, so s need not be a string */
size_t strnlen(const char *s, size_t maxlen)
{
	const char *end = memchr(s, '\0', maxlen);

	return end != nullptr ? (size_t)(end - s) : maxlen;
}
