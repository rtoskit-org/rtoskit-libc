/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_none_start.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "rtos-none.h"

/* The one thread's TLS block (docs/research/threads.md) */
static char tls_block[__RTOS_NONE_TLS_BLOCK_SIZE];

/* argv[argc] is a null pointer (C23 5.1.2.3.2p2) */
static char *arguments[1];

[[noreturn]] void __rtos_none_start(const struct __libc_tls_image *tls)
{
	if (__libc_tls_block_size(tls) > sizeof(tls_block))
		__rtos_abort();

	__rtos_none_set_tp(__libc_tls_init(tls_block, tls));

	/* The port runs .preinit_array (PLAN D104), with the TLS already in place */
	for (void (*const *entry)(void) = __preinit_array_start; entry < __preinit_array_end; ++entry)
		(*entry)();

	__libc_start_main(main, 0, arguments);
}
