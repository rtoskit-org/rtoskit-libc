/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * headers17.cpp
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * Every public header compiled as C++17 (the test compiles cxx.cpp as C++23):
 * the headers parse, and the functions they declare have C linkage, which
 * linking against libc.a proves (PLAN D120).
 */
#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbit.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <tgmath.h>
#include <threads.h>
#include <time.h>
#include <rtos/rtos-port.h>

#include "cxx.h"

int headers17(void)
{
	char buffer[16];
	once_flag flag = ONCE_FLAG_INIT;

	call_once(&flag, [] {});
	snprintf(buffer, sizeof(buffer), "%d", abs(-17));
	if (strcmp(buffer, "17") != 0 || toupper('a') != 'A' || stdc_count_ones_ui(7u) != 3 || (int)floor(2.5) != 2)
		return 1;
	/* Macros that expand in C++ too */
	int sum;
	struct pair {
		int first;
		int second;
	};

	assert(abs(-1) == 1);
	if (ckd_add(&sum, 40, 2) || sum != 42 || !isnan(nan("")) || signbit(-1.0) == 0 || offsetof(pair, second) != sizeof(int))
		return 1;

	return strtol("42", nullptr, 10) == 42 ? 0 : 1;
}
