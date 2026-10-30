/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * rtos-linux.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _RTOS_LINUX_H
#define _RTOS_LINUX_H

#include <rtos/rtos-port.h>

/* Linux port services beyond the seam, used by thread creation and the port tests */

/* The executable's TLS image, needed again for every new thread */
extern struct __libc_tls_image __rtos_linux_tls_image;

/* The environment passed by the kernel, kept for __rtos_getenv */
extern char **__rtos_linux_environ;

void __rtos_linux_set_tp(void *tp);
void *__rtos_linux_get_tp(void);

#endif /* _RTOS_LINUX_H */
