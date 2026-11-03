/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * unistd.h
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * POSIX.1-2024 <unistd.h>, the part of it the POSIX subset needs (PLAN
 * D128). A descriptor is the port's seam handle (D129), and the library's
 * definitions of read, write, close and lseek go through the seam; a port
 * with its own descriptor layer replaces them (D130).
 */
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ssize_t;
typedef long long off_t;
typedef int pid_t;

#define NULL __LIBC_NULL

/* The seam's handles 0, 1 and 2 */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* lseek's whence, the values <stdio.h> gives fseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
int close(int fildes);
off_t lseek(int fildes, off_t offset, int whence);

/* The one process the program is (P7.04, D134) */
pid_t getpid(void);

/*
 * The seam has no terminals and cannot duplicate a handle (issue #113, D144):
 * isatty answers 0 with ENOTTY for every valid descriptor, dup fails with
 * ENOSYS
 */
int isatty(int fildes);
int dup(int fildes);

__LIBC_END_DECLS

#endif /* _UNISTD_H */
