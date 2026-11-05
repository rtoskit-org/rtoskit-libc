/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * headers98.cpp
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The headers a C++98 translation unit can include, compiled as gnu++98,
 * the mode libstdc++ builds part of itself in (PLAN D131, issue #106): they
 * parse without C++11 syntax, errno is usable, and the functions have C
 * linkage. The C23-only headers (<stdbit.h>, <stdckdint.h>, <threads.h>,
 * <stdatomic.h>, <uchar.h>) have no C++98 counterpart and are left out.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <rtoskit-libc.h>
#include <sched.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cxx.h"

int headers98(void)
{
	char buffer[16];
	char *end = NULL;
	struct sched_param param;
	off_t offset = 0;

	param.sched_priority = 1;
	errno = 0;
	snprintf(buffer, sizeof(buffer), "%ld", strtol("98", &end, 10));
	if (errno != 0 || end == NULL || *end != '\0' || strcmp(buffer, "98") != 0)
		return 1;
	if (strcmp(strerror(EBADFD), "File descriptor in bad state") != 0 || toupper('a') != 'A' || (int)floor(2.5) != 2)
		return 1;
	if (STDIN_FILENO != 0 || STDOUT_FILENO != 1 || STDERR_FILENO != 2 || (O_RDWR & O_ACCMODE) != O_RDWR || sizeof(offset) != 8)
		return 1;
	if (SCHED_FIFO == SCHED_RR || param.sched_priority != 1 || sizeof(ssize_t) != sizeof(size_t))
		return 1;
	assert(abs(-1) == 1);

	return 0;
}
