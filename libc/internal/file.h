/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * file.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_FILE_H
#define _INTERNAL_FILE_H

#include <rtos/rtos-port.h>
#include <stddef.h>
#include <stdio.h>

/*
 * The stream object (docs/research/stdio.md). One buffer window serves both
 * directions: while reading, [pos, end) holds unread input; while writing,
 * [buffer, pos) holds pending output. stdin and stdout start with a static
 * buffer in their own objects; other streams get theirs at the first
 * transfer (malloc'd BUFSIZ bytes, a setvbuf buffer, or the one-byte
 * "single" for unbuffered streams). PLAN D53.
 */

/* Access, given by the mode */
#define __LIBC_FILE_READ 0x0001
#define __LIBC_FILE_WRITE 0x0002
/* Indicators (C23 7.23.1p2) */
#define __LIBC_FILE_EOF 0x0004
#define __LIBC_FILE_ERROR 0x0008
/* Buffering */
#define __LIBC_FILE_LINE 0x0010
#define __LIBC_FILE_UNBUFFERED 0x0020
#define __LIBC_FILE_OWN_BUFFER 0x0040
/* Current direction of the window, at most one */
#define __LIBC_FILE_READING 0x0080
#define __LIBC_FILE_WRITING 0x0100
/* A pushed-back character is held in ungot */
#define __LIBC_FILE_UNGOT 0x0200
/* The FILE itself came from malloc (fopen, tmpfile) */
#define __LIBC_FILE_ALLOCATED 0x0400
/* The buffer is the caller's, from setvbuf */
#define __LIBC_FILE_USER_BUFFER 0x0800

struct __libc_file {
	unsigned char *buffer;
	unsigned char *pos;
	unsigned char *end;
	size_t size;
	struct __rtos_lock *lock;
	struct __libc_file *next;
	int handle;
	unsigned short flags;
	unsigned char ungot;
	unsigned char single;
};

/* The streams fopen and tmpfile created, singly linked under __rtos_stdio_lock */
extern struct __libc_file *__libc_files;

static inline void __libc_file_lock(FILE *stream)
{
	__rtos_lock_acquire(stream->lock);
}

static inline void __libc_file_unlock(FILE *stream)
{
	__rtos_lock_release(stream->lock);
}

/*
 * The internal operations below expect the caller to hold the stream's lock.
 * Failures set the error indicator and errno.
 */

/* Set up the buffer if needed (falls back to unbuffered when malloc fails or is not linked) */
void __libc_file_setup(FILE *stream);

/* Write out pending output, or give back unread input; returns 0 or EOF */
int __libc_file_flush(FILE *stream);

/* Write count bytes; returns how many were accepted */
size_t __libc_file_write(FILE *stream, const void *buffer, size_t count);

/* Read up to count bytes; returns how many were read (short at end of file or error) */
size_t __libc_file_read(FILE *stream, void *buffer, size_t count);

/* One character as an unsigned char converted to int, or EOF */
int __libc_file_getc(FILE *stream);

/* Fill the empty read window from the handle; returns false at end of file or on error */
bool __libc_file_refill(FILE *stream);

/* The position as the stream sees it (pending output and unread input counted); -1 on error */
long long __libc_file_position(FILE *stream);

/*
 * C23 7.23.3p3: before input from an unbuffered or line-buffered stream, flush
 * every line-buffered output stream. Called without the stream's lock held,
 * because it takes the other streams' locks.
 */
void __libc_file_before_input(FILE *stream);

/*
 * Parse an fopen mode (C23 7.23.5.3): the __RTOS_OPEN_* flags and the stream's
 * access flags; returns false (with errno EINVAL) for an invalid mode.
 */
bool __libc_file_mode(const char *mode, unsigned int *open_flags, unsigned short *stream_flags);

/* Make a FILE for an open handle and link it; returns null (handle left open) on failure */
FILE *__libc_file_new(int handle, unsigned short flags);

void __libc_file_link(FILE *stream);
void __libc_file_unlink(FILE *stream);

/* Flush every stream (fflush(NULL)), or only the line-buffered ones; returns 0 or EOF */
int __libc_stdio_flush_all(bool line_buffered_only);

#endif /* _INTERNAL_FILE_H */
