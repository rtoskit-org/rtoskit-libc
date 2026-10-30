/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stdio.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.23 */
#define __STDC_VERSION_STDIO_H__ 202311L

typedef __SIZE_TYPE__ size_t;

typedef struct __libc_file FILE;

/* A position of a byte stream; there are no multibyte parse states to keep */
typedef struct {
	long long __offset;
} fpos_t;

#define NULL __LIBC_NULL

/* setvbuf modes (7.23.5.6) */
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

/* The default buffer size, the C minimum: RAM per stream matters more than speed (docs/research/stdio.md) */
#define BUFSIZ 256

#define EOF (-1)

#define FOPEN_MAX 16
#define FILENAME_MAX 256
#define L_tmpnam 32
#define TMP_MAX 10000

/* printf writes NaN as "nan" without an n-char-sequence (7.23.1p3) */
#define _PRINTF_NAN_LEN_MAX 0

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern FILE __libc_stdin;
extern FILE __libc_stdout;
extern FILE __libc_stderr;

#define stdin (&__libc_stdin)
#define stdout (&__libc_stdout)
#define stderr (&__libc_stderr)

/* Operations on files (7.23.4) */
int remove(const char *filename);
int rename(const char *old_name, const char *new_name);
FILE *tmpfile(void);
char *tmpnam(char *s);

/* File access functions (7.23.5) */
int fclose(FILE *stream);
int fflush(FILE *stream);
FILE *fopen(const char *__restrict filename, const char *__restrict mode);
FILE *freopen(const char *__restrict filename, const char *__restrict mode, FILE *__restrict stream);
void setbuf(FILE *__restrict stream, char *__restrict buf);
int setvbuf(FILE *__restrict stream, char *__restrict buf, int mode, size_t size);

/* Formatted input/output functions (7.23.6) */
int fprintf(FILE *__restrict stream, const char *__restrict format, ...);
int fscanf(FILE *__restrict stream, const char *__restrict format, ...);
int printf(const char *__restrict format, ...);
int scanf(const char *__restrict format, ...);
int snprintf(char *__restrict s, size_t n, const char *__restrict format, ...);
int sprintf(char *__restrict s, const char *__restrict format, ...);
int sscanf(const char *__restrict s, const char *__restrict format, ...);
int vfprintf(FILE *__restrict stream, const char *__restrict format, __builtin_va_list arg);
int vfscanf(FILE *__restrict stream, const char *__restrict format, __builtin_va_list arg);
int vprintf(const char *__restrict format, __builtin_va_list arg);
int vscanf(const char *__restrict format, __builtin_va_list arg);
int vsnprintf(char *__restrict s, size_t n, const char *__restrict format, __builtin_va_list arg);
int vsprintf(char *__restrict s, const char *__restrict format, __builtin_va_list arg);
int vsscanf(const char *__restrict s, const char *__restrict format, __builtin_va_list arg);

/* Character input/output functions (7.23.7) */
int fgetc(FILE *stream);
char *fgets(char *__restrict s, int n, FILE *__restrict stream);
int fputc(int c, FILE *stream);
int fputs(const char *__restrict s, FILE *__restrict stream);
int getc(FILE *stream);
int getchar(void);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(const char *s);
int ungetc(int c, FILE *stream);

/* Direct input/output functions (7.23.8) */
size_t fread(void *__restrict ptr, size_t size, size_t nmemb, FILE *__restrict stream);
size_t fwrite(const void *__restrict ptr, size_t size, size_t nmemb, FILE *__restrict stream);

/* File positioning functions (7.23.9) */
int fgetpos(FILE *__restrict stream, fpos_t *__restrict pos);
int fseek(FILE *stream, long int offset, int whence);
int fsetpos(FILE *stream, const fpos_t *pos);
long int ftell(FILE *stream);
void rewind(FILE *stream);

/* Error-handling functions (7.23.10) */
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);

__LIBC_END_DECLS

#endif /* _STDIO_H */
