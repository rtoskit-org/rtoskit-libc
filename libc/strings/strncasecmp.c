/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strncasecmp.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <strings.h>

#include "internal/ctype.h"

/*
 * POSIX.1-2024 strncasecmp: compares as if both strings were lower case, in the
 * "C" locale (the only one, PLAN D47), with the bytes taken as unsigned char
 */
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	for (; n > 0; --n) {
		int c1 = (unsigned char)*s1++;
		int c2 = (unsigned char)*s2++;

		if (__libc_isupper(c1))
			c1 |= 0x20;
		if (__libc_isupper(c2))
			c2 |= 0x20;
		if (c1 != c2 || c1 == '\0')
			return c1 - c2;
	}

	return 0;
}
