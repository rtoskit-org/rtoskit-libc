/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * types.h
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/*
 * POSIX.1-2024 <sys/types.h>, the part of it the POSIX subset needs (PLAN
 * D128). Each type is the same on every core of a target triple (D15).
 */
typedef __SIZE_TYPE__ size_t;

/* A signed type the width of size_t: ptrdiff_t is that on every target */
typedef __PTRDIFF_TYPE__ ssize_t;

/* File offsets are 64 bits everywhere, as __rtos_seek's are (D129) */
typedef long long off_t;

typedef unsigned int mode_t;
typedef int pid_t;

/* The clock argument the seam's __rtos_clock_gettime takes */
typedef int clockid_t;

/* struct stat's fields (<sys/stat.h>, P7.04): Linux's widths, the same on every core */
typedef unsigned long long dev_t;
typedef unsigned long long ino_t;
typedef unsigned int nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long blksize_t;
typedef long long blkcnt_t;

/* The threads types (phase 10, PLAN D148), shared with <pthread.h> */
#include <__pthread_types.h>

#endif /* _SYS_TYPES_H */
