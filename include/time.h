/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * time.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _TIME_H
#define _TIME_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.29 */
#define __STDC_VERSION_TIME_H__ 202311L

typedef __SIZE_TYPE__ size_t;

#define NULL __LIBC_NULL

/* Processor time in microseconds (docs/research/time-locale.md) */
#define CLOCKS_PER_SEC ((clock_t)1000000)

/* Time bases for timespec_get and timespec_getres (7.29.1p3); the last three are optional and supported */
#define TIME_UTC 1
#define TIME_MONOTONIC 2
#define TIME_ACTIVE 3
#define TIME_THREAD_ACTIVE 4

typedef long long clock_t;

/* 64-bit seconds on every target: no year 2038 limit, one header for all (PLAN D15) */
typedef long long time_t;

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

/* Time manipulation functions (7.29.2) */
clock_t clock(void);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
time_t timegm(struct tm *timeptr);
time_t time(time_t *timer);
int timespec_get(struct timespec *ts, int base);
int timespec_getres(struct timespec *ts, int base);

/* Time conversion functions (7.29.3) */
[[deprecated]] char *asctime(const struct tm *timeptr);
[[deprecated]] char *ctime(const time_t *timer);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *timer, struct tm *buf);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *timer, struct tm *buf);
size_t strftime(char *__restrict s, size_t maxsize, const char *__restrict format, const struct tm *__restrict timeptr);

__LIBC_END_DECLS

#endif /* _TIME_H */
