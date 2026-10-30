/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * string.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STRING_H
#define _STRING_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.26 */
#define __STDC_VERSION_STRING_H__ 202311L

typedef __SIZE_TYPE__ size_t;

#define NULL __LIBC_NULL

/* Copying functions (C23 7.26.2) */
void *memcpy(void *__restrict s1, const void *__restrict s2, size_t n);
void *memccpy(void *__restrict s1, const void *__restrict s2, int c, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
char *strcpy(char *__restrict s1, const char *__restrict s2);
char *strdup(const char *s);
char *strncpy(char *__restrict s1, const char *__restrict s2, size_t n);
char *strndup(const char *s, size_t size);

/* Concatenation functions (C23 7.26.3) */
char *strcat(char *__restrict s1, const char *__restrict s2);
char *strncat(char *__restrict s1, const char *__restrict s2, size_t n);

/* Comparison functions (C23 7.26.4) */
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strcoll(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strxfrm(char *__restrict s1, const char *__restrict s2, size_t n);

/* Search functions (C23 7.26.5) */
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *__restrict s1, const char *__restrict s2);

/* Miscellaneous functions (C23 7.26.6) */
void *memset(void *s, int c, size_t n);
void *memset_explicit(void *s, int c, size_t n);
char *strerror(int errnum);
size_t strlen(const char *s);

/*
 * C23 7.26.5.1: the search functions are generic and return a pointer to a
 * const-qualified type when the first argument points to one. A conditional
 * with void * yields void * carrying the pointed-to qualifiers, which
 * _Generic then selects on; the controlling expression is not evaluated.
 */
#ifndef __cplusplus
#define __LIBC_CONST_GENERIC(p, type, call) _Generic(0 ? (p) : (void *)1, const void *: (const type *)(call), default: (call))

#define memchr(s, c, n) __LIBC_CONST_GENERIC(s, void, memchr(s, c, n))
#define strchr(s, c) __LIBC_CONST_GENERIC(s, char, strchr(s, c))
#define strpbrk(s1, s2) __LIBC_CONST_GENERIC(s1, char, strpbrk(s1, s2))
#define strrchr(s, c) __LIBC_CONST_GENERIC(s, char, strrchr(s, c))
#define strstr(s1, s2) __LIBC_CONST_GENERIC(s1, char, strstr(s1, s2))
#endif

__LIBC_END_DECLS

#endif /* _STRING_H */
