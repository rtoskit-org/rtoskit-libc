/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __once_flag.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef ___ONCE_FLAG_H
#define ___ONCE_FLAG_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * once_flag and call_once, declared by both <threads.h> (C23 7.28.1) and
 * <stdlib.h> (7.24); this piece keeps the type defined once. The state is
 * 0 not run, 1 running, 2 running with waiters, 3 done
 * (docs/research/threads.md).
 */
typedef struct {
	unsigned int __state;
} once_flag;

#define ONCE_FLAG_INIT \
	{                  \
		0              \
	}

void call_once(once_flag *flag, void (*func)(void));

__LIBC_END_DECLS

#endif /* ___ONCE_FLAG_H */
