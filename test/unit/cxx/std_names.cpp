/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * std_names.cpp
 *
 * Created on: Nov 6, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The headers libstdc++ reaches (its <c*> wrappers, and gthr-posix.h's
 * <pthread.h>) must not declare a global name that C++ declares differently
 * in namespace std, or "using namespace std;" makes the name ambiguous
 * (issue #138, PLAN D161). Every public header but <threads.h>, which C++
 * does not have and a program includes only to get C's names, is included
 * here; the names namespace std holds are then declared as stand-ins for
 * libstdc++'s and used unqualified.
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
#include <time.h>
#include <rtoskit-libc.h>
#include <rtos/rtos-port.h>

/* The POSIX subset (PLAN D128) and the threads gthr-posix.h uses (D148) */
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cxx.h"

/* Stand-ins for <mutex>'s and <utility>'s declarations */
namespace std
{
struct once_flag {
	int state;
};

template <class callable>
void call_once(once_flag &flag, callable function)
{
	if (flag.state == 0) {
		flag.state = 1;
		function();
	}
}

[[noreturn]] inline void unreachable()
{
	__builtin_unreachable();
}
}

using namespace std;

/* regression_once_flag_ambiguous: <stdlib.h> declared C's ::once_flag and ::call_once in C++ (issue #138) */
static once_flag regression_once_flag;

int std_names(void)
{
	int calls = 0;

	call_once(regression_once_flag, [&calls] { ++calls; });
	call_once(regression_once_flag, [&calls] { ++calls; });
	if (calls != 1)
		unreachable();

	return calls;
}
