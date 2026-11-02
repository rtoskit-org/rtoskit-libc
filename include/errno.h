/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * errno.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _ERRNO_H
#define _ERRNO_H

/*
 * C23 7.5. The values follow the Linux numbering so the Linux test port can
 * pass kernel error numbers through unchanged (PLAN D20).
 */
#define EDOM 33
#define ERANGE 34
#define EILSEQ 84

/* Additional error numbers (C23 7.5p4): the POSIX set, Linux values */
#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define ENOMSG 42
#define EIDRM 43
#define ENOSTR 60
#define ENODATA 61
#define ETIME 62
#define ENOSR 63
#define ENOLINK 67
#define EPROTO 71
#define EMULTIHOP 72
#define EBADMSG 74
#define EOVERFLOW 75
#define ENOTSOCK 88
#define EDESTADDRREQ 89
#define EMSGSIZE 90
#define EPROTOTYPE 91
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define EOPNOTSUPP 95
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define EADDRNOTAVAIL 99
#define ENETDOWN 100
#define ENETUNREACH 101
#define ENETRESET 102
#define ECONNABORTED 103
#define ECONNRESET 104
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ETIMEDOUT 110
#define ECONNREFUSED 111
#define EHOSTUNREACH 113
#define EALREADY 114
#define EINPROGRESS 115
#define ESTALE 116
#define EDQUOT 122
#define ECANCELED 125
#define EOWNERDEAD 130
#define ENOTRECOVERABLE 131

/*
 * Beyond POSIX: the BSD and Linux codes a consumer of the POSIX subset uses
 * (PLAN D128), with Linux values. EFTYPE (BSD) has no Linux number; 200 lies
 * above every Linux value (133 is the last) and fits strerror's table.
 */
#define ENOTUNIQ 76
#define EBADFD 77
#define ESOCKTNOSUPPORT 94
#define ESHUTDOWN 108
#define EHOSTDOWN 112
#define EFTYPE 200

/* Aliases POSIX allows to share a value */
#define ENOTSUP EOPNOTSUPP
#define EWOULDBLOCK EAGAIN

/*
 * C23 7.5p2: a modifiable lvalue of type int with thread storage duration.
 * Spelt __thread, which GCC and Clang take in every C and C++ mode, C++98
 * included (PLAN D131): libstdc++ compiles part of itself as C++98.
 */
extern __thread int errno;
#define errno errno

#endif /* _ERRNO_H */
