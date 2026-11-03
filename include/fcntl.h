/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * fcntl.h
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FCNTL_H
#define _FCNTL_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * POSIX.1-2024 <fcntl.h>, the part of it the POSIX subset needs (PLAN D128).
 * The flag values are Linux's, as errno's are (D20), so the Linux test port
 * and a port with a POSIX layer of its own see the numbers they expect; the
 * library's open translates them to the seam's __RTOS_OPEN_* flags (D130).
 */
typedef unsigned int mode_t;
typedef long long off_t;
typedef int pid_t;

/* The access mode: exactly one of these */
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_ACCMODE 03

/* File creation flags */
#define O_CREAT 0100
#define O_EXCL 0200
#define O_TRUNC 01000

/* File status flags */
#define O_APPEND 02000
#define O_NONBLOCK 04000

/*
 * Close on exec (P7.04, D134): nothing here executes another program, so the
 * flag is accepted and has no effect; libbacktrace sets it when it opens a
 * file
 */
#define O_CLOEXEC 02000000

/* fcntl's commands: the descriptor flags only */
#define F_GETFD 1
#define F_SETFD 2
#define FD_CLOEXEC 1

/* The whence values <stdio.h> gives fseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int open(const char *path, int oflag, ...);
int fcntl(int fildes, int cmd, ...);

__LIBC_END_DECLS

#endif /* _FCNTL_H */
