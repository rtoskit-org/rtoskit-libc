/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strings.h
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STRINGS_H
#define _STRINGS_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * POSIX.1-2024 <strings.h>, the part of it the POSIX subset needs (PLAN
 * D128): case-insensitive comparison in the "C" locale
 */
typedef __SIZE_TYPE__ size_t;

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

__LIBC_END_DECLS

#endif /* _STRINGS_H */
