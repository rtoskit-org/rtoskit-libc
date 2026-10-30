/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * feclearexcept.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <fenv.h>

#include "fenv-arch.h"

/* C23 7.6.4.1; bits that name no supported exception are ignored and the call succeeds */
int feclearexcept(int excepts)
{
	unsigned int state;

	excepts &= FE_ALL_EXCEPT;
	state = __libc_fenv_read();
	__libc_fenv_write(__libc_fenv_with_flags(state, __libc_fenv_flags(state) & ~(unsigned int)excepts));

	return 0;
}
