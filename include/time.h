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
__attribute__((__deprecated__)) char *asctime(const struct tm *timeptr);
__attribute__((__deprecated__)) char *ctime(const time_t *timer);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *timer, struct tm *buf);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *timer, struct tm *buf);
size_t strftime(char *__restrict s, size_t maxsize, const char *__restrict format, const struct tm *__restrict timeptr);

/*
 * POSIX.1-2024 clocks (PLAN D128, D132): a clockid_t is a seam time base, so
 * CLOCK_REALTIME is TIME_UTC and CLOCK_MONOTONIC is TIME_MONOTONIC, as the
 * ISO bases already mean
 */
#if __LIBC_POSIX_VISIBLE
typedef int clockid_t;

#define CLOCK_REALTIME 1
#define CLOCK_MONOTONIC 2
#define CLOCK_PROCESS_CPUTIME_ID 3
#define CLOCK_THREAD_CPUTIME_ID 4

int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_getres(clockid_t clock_id, struct timespec *res);
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
#endif

__LIBC_END_DECLS

#endif /* _TIME_H */
