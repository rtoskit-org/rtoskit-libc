/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __cdefs.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef ___CDEFS_H
#define ___CDEFS_H

/*
 * What the public headers need to compile as C++ as well as C (PLAN D120,
 * issue #98): C linkage for every declaration, and a null pointer constant
 * C++ accepts. The headers spell restrict and typeof as __restrict and
 * __typeof__, which GCC and Clang take in both languages, so these are the
 * only conditionals the language needs.
 */
#ifdef __cplusplus
#define __LIBC_BEGIN_DECLS extern "C" {
#define __LIBC_END_DECLS }
#define __LIBC_NULL nullptr
#else
#define __LIBC_BEGIN_DECLS
#define __LIBC_END_DECLS
#define __LIBC_NULL ((void *)0)
#endif

#endif /* ___CDEFS_H */
