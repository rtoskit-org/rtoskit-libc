/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * assert.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _ASSERT_H
#define _ASSERT_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.2 */
#define __STDC_VERSION_ASSERT_H__ 202311L

[[noreturn]] void __libc_assert_fail(const char *expression, const char *file, int line, const char *function);

/* Takes exactly one scalar, so assert() and assert(1, 1) are diagnosed (C23 7.2.1p3) */
static inline bool __libc_assert_scalar(bool value)
{
	return value;
}

__LIBC_END_DECLS

#endif /* _ASSERT_H */

/* C23 7.2.1p1: redefined on every inclusion according to NDEBUG, so outside the guard */
#undef assert
#ifdef NDEBUG
#define assert(...) ((void)0)
#else
#define assert(...) (__libc_assert_scalar(__VA_ARGS__) ? (void)0 : __libc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__, __func__))
#endif
