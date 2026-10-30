/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stdlib.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.24 */
#define __STDC_VERSION_STDLIB_H__ 202311L

typedef __SIZE_TYPE__ size_t;
/* wchar_t is a keyword in C++ */
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif

#define NULL __LIBC_NULL

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/* PCG32's top 31 bits (docs/research/stdlib-part2.md) */
#define RAND_MAX 0x7fffffff

/* The "C" locale is single-byte (C23 7.24p3) */
#define MB_CUR_MAX ((size_t)1)

/* once_flag, ONCE_FLAG_INIT and call_once (7.24p2, also in <threads.h>) */
#include <__once_flag.h>

typedef struct {
	int quot;
	int rem;
} div_t;

typedef struct {
	long int quot;
	long int rem;
} ldiv_t;

typedef struct {
	long long int quot;
	long long int rem;
} lldiv_t;

/* Numeric conversion functions (C23 7.24.1) */
double atof(const char *nptr);
int atoi(const char *nptr);
long int atol(const char *nptr);
long long int atoll(const char *nptr);
double strtod(const char *__restrict nptr, char **__restrict endptr);
float strtof(const char *__restrict nptr, char **__restrict endptr);
long double strtold(const char *__restrict nptr, char **__restrict endptr);
int strfromd(char *__restrict s, size_t n, const char *__restrict format, double fp);
int strfromf(char *__restrict s, size_t n, const char *__restrict format, float fp);
int strfroml(char *__restrict s, size_t n, const char *__restrict format, long double fp);
long int strtol(const char *__restrict nptr, char **__restrict endptr, int base);
long long int strtoll(const char *__restrict nptr, char **__restrict endptr, int base);
unsigned long int strtoul(const char *__restrict nptr, char **__restrict endptr, int base);
unsigned long long int strtoull(const char *__restrict nptr, char **__restrict endptr, int base);

/* Pseudo-random sequence generation functions (C23 7.24.2) */
int rand(void);
void srand(unsigned int seed);

/* Communication with the environment (C23 7.24.4) */
[[noreturn]] void abort(void);
int atexit(void (*func)(void));
int at_quick_exit(void (*func)(void));
[[noreturn]] void exit(int status);
[[noreturn]] void _Exit(int status);
char *getenv(const char *name);
[[noreturn]] void quick_exit(int status);
int system(const char *string);

/* Searching and sorting utilities (C23 7.24.5) */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* Integer arithmetic functions (C23 7.24.6) */
int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);
div_t div(int numer, int denom);
ldiv_t ldiv(long int numer, long int denom);
lldiv_t lldiv(long long int numer, long long int denom);

/* Multibyte/wide character conversion functions (C23 7.24.7, 7.24.8), "C" locale */
int mblen(const char *s, size_t n);
int mbtowc(wchar_t *__restrict pwc, const char *__restrict s, size_t n);
int wctomb(char *s, wchar_t wc);
size_t mbstowcs(wchar_t *__restrict pwcs, const char *__restrict s, size_t n);
size_t wcstombs(char *__restrict s, const wchar_t *__restrict pwcs, size_t n);

/* Memory management functions (C23 7.24.3) */
void *aligned_alloc(size_t alignment, size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void free_sized(void *ptr, size_t size);
void free_aligned_sized(void *ptr, size_t alignment, size_t size);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

/* Alignment of memory (C23 7.24.9) */
size_t memalignment(const void *p);

/* C23 7.24.5.1p4: bsearch returns a pointer to const when base points to const (as in <string.h>) */
#ifndef __cplusplus
#define __LIBC_CONST_GENERIC(p, type, call) _Generic(0 ? (p) : (void *)1, const void *: (const type *)(call), default: (call))
#define bsearch(key, base, nmemb, size, compar) __LIBC_CONST_GENERIC(base, void, bsearch(key, base, nmemb, size, compar))
#endif

__LIBC_END_DECLS

#endif /* _STDLIB_H */
