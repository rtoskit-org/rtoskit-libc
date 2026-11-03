/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * getpid.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <unistd.h>

/*
 * POSIX.1-2024 getpid: the program is the only process, so its ID is a
 * constant; 1 because 0 is not a process ID on POSIX systems (P7.04, D134)
 */
pid_t getpid(void)
{
	return 1;
}
