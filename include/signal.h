/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * signal.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.14: software signals only (raise), no asynchronous OS signals */
typedef int sig_atomic_t;

/* Distinct values no function can have; SIG_ERR is what signal returns on error */
#define SIG_DFL ((void (*)(int))0)
#define SIG_ERR ((void (*)(int)) - 1)
#define SIG_IGN ((void (*)(int))1)

/* Linux numbering (PLAN D20) */
#define SIGINT 2
#define SIGILL 4
#define SIGABRT 6
#define SIGFPE 8
#define SIGSEGV 11
#define SIGTERM 15

void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

__LIBC_END_DECLS

#endif /* _SIGNAL_H */
