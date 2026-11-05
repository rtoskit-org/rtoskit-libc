/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_entropy.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>

#include "linux.h"

/*
 * getrandom(2) with no flags blocks until the kernel's pool is initialised;
 * it may return fewer bytes than asked when a signal arrives, so loop until
 * count bytes are written
 */
int __rtos_entropy(void *buffer, size_t count)
{
	unsigned char *next = buffer;

	while (count > 0) {
		long status = __linux_syscall3(__LINUX_SYS_GETRANDOM, (long)next, (long)count, 0);

		if (status == -EINTR)
			continue;
		if (status < 0)
			return (int)status;
		if (status == 0)
			return -EIO;
		next += status;
		count -= (size_t)status;
	}

	return 0;
}
