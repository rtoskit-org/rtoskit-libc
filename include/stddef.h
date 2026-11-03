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
/*
 * wchar_t is a keyword in C++. nullptr arrived in C++11, so C++98 has no
 * nullptr_t (PLAN D131)
 */
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
typedef __typeof__(nullptr) nullptr_t;
#elif __cplusplus >= 201103L
typedef decltype(nullptr) nullptr_t;
#endif

/* The greatest fundamental alignment is that of the widest scalar types */
typedef struct {
	long long __ll __attribute__((__aligned__(__alignof__(long long))));
	long double __ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;

#define NULL __LIBC_NULL

/*
 * C23 7.21.1; C++ has std::unreachable() in <utility> instead, which this
 * macro would rewrite (issue #113, D120, D144)
 */
#ifndef __cplusplus
#define unreachable() __builtin_unreachable()
#endif

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
