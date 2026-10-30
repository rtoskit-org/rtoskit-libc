/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stddef.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDDEF_H
#define _STDDEF_H

#include <__cdefs.h>

/* C23 7.21 */
#define __STDC_VERSION_STDDEF_H__ 202311L

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
/* wchar_t is a keyword in C++ */
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif
typedef __typeof__(nullptr) nullptr_t;

/* The greatest fundamental alignment is that of the widest scalar types */
typedef struct {
	alignas(long long) long long __ll;
	alignas(long double) long double __ld;
} max_align_t;

#define NULL __LIBC_NULL

#define unreachable() __builtin_unreachable()

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
