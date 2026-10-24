/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_start_main.c
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdlib.h>
#include <rtos/rtos-port.h>

#include "internal/sections.h"

/* C23 5.1.2.3.2; .preinit_array is the port's to run before this (rtos-port.h, PLAN D104) */
void __libc_start_main(int (*main)(int, char **), int argc, char **argv)
{
	for (void (*const *entry)(void) = __init_array_start; entry < __init_array_end; ++entry)
		(*entry)();
	exit(main(argc, argv));
}
