/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * call_once.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <threads.h>

#include "internal/once.h"

/*
 * C23 7.28.2.1. Built without unwind tables, so a C program pays nothing
 * for exceptions; C++'s std::call_once uses __libc_call_once_eh, which
 * resets the flag when the function throws (issue #121, PLAN D145).
 */
void call_once(once_flag *flag, void (*func)(void))
{
	if (!__libc_once_claim(&flag->__state))
		return;

	func();
	__libc_once_finish(&flag->__state, __LIBC_ONCE_DONE);
}
