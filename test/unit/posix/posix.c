/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * posix.c
 *
 * Created on: Nov 2, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/* The POSIX subset's names in the ISO headers (PLAN D132) */
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/*
 * The POSIX subset's functions (P7.03, PLAN D128-D130, D132,
 * docs/research/posix-subset.md): descriptors as seam handles through
 * open, read, write, lseek, close, fileno and fdopen, on the Linux port's
 * files; strnlen, strcasecmp, strncasecmp, reallocarray, sincos and
 * sincosf; and clock_gettime, clock_getres, nanosleep and sched_yield.
 */

static int failures;

static void fail(int line, const char *what)
{
	printf("fail: line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

static long long nanoseconds(const struct timespec *ts)
{
	return ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

static void check_descriptors(void)
{
	char name[L_tmpnam];
	char buffer[16];
	int fd;

	CHECK(tmpnam(name) != nullptr);

	/* open, write, lseek, read and close round trip */
	fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
	CHECK(fd > STDERR_FILENO);
	CHECK(write(fd, "hello, world", 12) == 12);
	CHECK(lseek(fd, 0, SEEK_CUR) == 12 && lseek(fd, 0, SEEK_END) == 12);
	CHECK(lseek(fd, 7, SEEK_SET) == 7);
	CHECK(read(fd, buffer, sizeof(buffer)) == 5 && memcmp(buffer, "world", 5) == 0);
	CHECK(read(fd, buffer, sizeof(buffer)) == 0);
	errno = 0;
	CHECK(lseek(fd, -1, SEEK_SET) == -1 && errno == EINVAL);
	CHECK(close(fd) == 0);

	/* After close the descriptor is gone */
	errno = 0;
	CHECK(close(fd) == -1 && errno == EBADF);
	errno = 0;
	CHECK(read(fd, buffer, 1) == -1 && errno == EBADF);
	errno = 0;
	CHECK(write(fd, "x", 1) == -1 && errno == EBADF);

	/* O_EXCL on an existing file, a missing file, an invalid access mode */
	errno = 0;
	CHECK(open(name, O_WRONLY | O_CREAT | O_EXCL, 0600) == -1 && errno == EEXIST);
	errno = 0;
	CHECK(open("posix-missing-file", O_RDONLY) == -1 && errno == ENOENT);
	errno = 0;
	CHECK(open(name, O_ACCMODE) == -1 && errno == EINVAL);

	/* O_APPEND writes at the end; O_RDONLY cannot write */
	fd = open(name, O_WRONLY | O_APPEND);
	CHECK(fd >= 0 && lseek(fd, 0, SEEK_SET) == 0 && write(fd, "!", 1) == 1 && lseek(fd, 0, SEEK_CUR) == 13 && close(fd) == 0);
	fd = open(name, O_RDONLY);
	errno = 0;
	CHECK(fd >= 0 && write(fd, "x", 1) == -1 && errno == EBADF);
	CHECK(read(fd, buffer, sizeof(buffer)) == 13 && memcmp(buffer, "hello, world!", 13) == 0 && close(fd) == 0);

	/* O_TRUNC empties the file */
	fd = open(name, O_RDWR | O_TRUNC);
	CHECK(fd >= 0 && lseek(fd, 0, SEEK_END) == 0 && close(fd) == 0);

	CHECK(remove(name) == 0);
}

/* fstat, fcntl and getpid (P7.04, PLAN D134) */
static void check_file_status(void)
{
	char name[L_tmpnam];
	struct stat st;
	int fd;

	CHECK(tmpnam(name) != nullptr);
	fd = open(name, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
	CHECK(fd >= 0);

	/* A regular file's size, with the offset left where it was */
	CHECK(fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 0);
	CHECK(write(fd, "0123456789", 10) == 10 && lseek(fd, 3, SEEK_SET) == 3);
	CHECK(fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode) && !S_ISCHR(st.st_mode) && st.st_size == 10);
	CHECK(lseek(fd, 0, SEEK_CUR) == 3);

	/* Descriptor flags: accepted, with no effect */
	CHECK(fcntl(fd, F_GETFD) == 0 && fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	errno = 0;
	CHECK(fcntl(fd, 99) == -1 && errno == EINVAL);
	CHECK(close(fd) == 0);

	/* A closed or negative descriptor */
	errno = 0;
	CHECK(fstat(fd, &st) == -1 && errno == EBADF);
	errno = 0;
	CHECK(fstat(-1, &st) == -1 && errno == EBADF);
	errno = 0;
	CHECK(fcntl(fd, F_GETFD) == -1 && errno == EBADF);
	errno = 0;
	CHECK(fcntl(-1, F_SETFD, FD_CLOEXEC) == -1 && errno == EBADF);

	/* The one process */
	CHECK(getpid() == 1 && getpid() == getpid());

	CHECK(remove(name) == 0);
}

static void check_streams(void)
{
	char name[L_tmpnam];
	char buffer[16] = { 0 };
	FILE *first;
	FILE *second;
	int fd;

	CHECK(fileno(stdin) == STDIN_FILENO && fileno(stdout) == STDOUT_FILENO && fileno(stderr) == STDERR_FILENO);

	/* Two streams on one descriptor: writes through one are read back through the other after fflush */
	CHECK(tmpnam(name) != nullptr);
	first = fopen(name, "w+");
	CHECK(first != nullptr);
	fd = fileno(first);
	CHECK(fd > STDERR_FILENO);
	second = fdopen(fd, "r+");
	CHECK(second != nullptr && fileno(second) == fd);
	CHECK(fputs("stream", first) >= 0 && fflush(first) == 0);
	CHECK(lseek(fd, 0, SEEK_END) == 6);
	CHECK(fseek(second, 0, SEEK_SET) == 0 && fgets(buffer, sizeof(buffer), second) != nullptr && strcmp(buffer, "stream") == 0);
	CHECK(fclose(second) == 0);

	/* fclose of the fdopen stream closed the shared descriptor */
	errno = 0;
	CHECK(lseek(fd, 0, SEEK_SET) == -1 && errno == EBADF);
	fclose(first);

	/* fdopen's "w" does not truncate */
	fd = open(name, O_RDWR);
	CHECK(fd >= 0);
	first = fdopen(fd, "w");
	CHECK(first != nullptr && fclose(first) == 0);
	fd = open(name, O_RDONLY);
	CHECK(fd >= 0 && lseek(fd, 0, SEEK_END) == 6 && close(fd) == 0);

	/* Invalid descriptors and modes */
	errno = 0;
	CHECK(fdopen(-1, "r") == nullptr && errno == EBADF);

	CHECK(remove(name) == 0);
}

/* fdopen failing leaves the descriptor open for its caller (POSIX.1-2024 fdopen) */
static void regression_fdopen_failure_keeps_descriptor(void)
{
	char name[L_tmpnam];
	int fd;

	CHECK(tmpnam(name) != nullptr);
	fd = open(name, O_RDWR | O_CREAT, 0600);
	CHECK(fd >= 0);
	errno = 0;
	CHECK(fdopen(fd, "q") == nullptr && errno == EINVAL);
	CHECK(write(fd, "x", 1) == 1 && close(fd) == 0);
	CHECK(remove(name) == 0);
}

static void check_strings(void)
{
	static const char bytes[] = { 'a', 'b', 'c' };

	/* strnlen stops at maxlen and needs no terminator within it */
	CHECK(strnlen("", 0) == 0 && strnlen("", 5) == 0);
	CHECK(strnlen("abc", 0) == 0 && strnlen("abc", 2) == 2 && strnlen("abc", 3) == 3 && strnlen("abc", 4) == 3 && strnlen("abc", SIZE_MAX) == 3);
	CHECK(strnlen(bytes, sizeof(bytes)) == 3);

	/* Case-insensitive in the "C" locale, bytes compared as unsigned char */
	CHECK(strcasecmp("Hello", "hELLO") == 0 && strcasecmp("", "") == 0);
	CHECK(strcasecmp("abc", "ABD") < 0 && strcasecmp("ABD", "abc") > 0);
	CHECK(strcasecmp("ab", "ABC") < 0 && strcasecmp("ABC", "ab") > 0);
	CHECK(strcasecmp("[", "a") < 0 && strcasecmp("[", "A") < 0);
	CHECK(strcasecmp("\xc4", "a") > 0 && strcasecmp("\xc4", "\xe4") != 0);
	CHECK(strcasecmp("@", "`") < 0);
	CHECK(strncasecmp("HelloX", "hELLOy", 5) == 0 && strncasecmp("abc", "ABD", 2) == 0 && strncasecmp("abc", "ABD", 3) < 0);
	CHECK(strncasecmp("a", "b", 0) == 0 && strncasecmp("ab", "AB", SIZE_MAX) == 0 && strncasecmp("ab", "abc", 3) < 0);
	CHECK(strncasecmp("\xff", "\x01", 1) > 0);
}

static void check_reallocarray(void)
{
	int *array = reallocarray(nullptr, 4, sizeof(int));
	int *grown;

	CHECK(array != nullptr);
	for (int i = 0; i < 4; ++i)
		array[i] = i * 11;
	grown = reallocarray(array, 1000, sizeof(int));
	CHECK(grown != nullptr && grown[0] == 0 && grown[3] == 33);
	if (grown != nullptr)
		array = grown;

	/* A product that wraps around fails with ENOMEM and keeps the block */
	errno = 0;
	CHECK(reallocarray(array, SIZE_MAX / 2 + 1, 2) == nullptr && errno == ENOMEM);
	errno = 0;
	CHECK(reallocarray(array, 2, SIZE_MAX / 2 + 1) == nullptr && errno == ENOMEM);
	CHECK(array[3] == 33);
	free(array);
}

/* The rounding directions the core has (C23 7.6p8): directed ones only with a double FPU or SSE (D44) */
#ifdef FE_UPWARD
static const int modes[] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
#else
static const int modes[] = { FE_TONEAREST };
#endif
static const size_t mode_count = sizeof(modes) / sizeof(modes[0]);

/* sincos against sin and cos: the same bits and the same exception flags */
static bool same_double(double x)
{
	double s;
	double c;
	double want_s;
	double want_c;
	int want_flags;

	feclearexcept(FE_ALL_EXCEPT);
	want_s = sin(x);
	want_c = cos(x);
	want_flags = fetestexcept(FE_ALL_EXCEPT);
	feclearexcept(FE_ALL_EXCEPT);
	sincos(x, &s, &c);
	return memcmp(&s, &want_s, sizeof(s)) == 0 && memcmp(&c, &want_c, sizeof(c)) == 0 && fetestexcept(FE_ALL_EXCEPT) == want_flags;
}

static bool same_float(float x)
{
	float s;
	float c;
	float want_s;
	float want_c;
	int want_flags;

	feclearexcept(FE_ALL_EXCEPT);
	want_s = sinf(x);
	want_c = cosf(x);
	want_flags = fetestexcept(FE_ALL_EXCEPT);
	feclearexcept(FE_ALL_EXCEPT);
	sincosf(x, &s, &c);
	return memcmp(&s, &want_s, sizeof(s)) == 0 && memcmp(&c, &want_c, sizeof(c)) == 0 && fetestexcept(FE_ALL_EXCEPT) == want_flags;
}

/*
 * sincos and sincosf share one reduction (P8.02) and must stay bit-identical
 * to sin, cos, sinf and cosf (P7.03): specials, tiny arguments, every
 * quadrant of each reduction regime (none below pi/4, Cody-Waite, Payne-Hanek
 * from 2^20), the doubles and floats nearest multiples of pi/2, sinf's and
 * cosf's hard cases, a sweep of exponents, in every rounding direction
 */
static void check_sincos(void)
{
	static const double doubles[] = { 0.0, -0.0, 0x1p-1074, 0x1p-1022, 0x1p-28, -0x1p-27, 0x1.fffffffffffffp-28, 0.5, 1.0, -1.0, 2.0, 3.0, -4.0, 5.0, 3.141592653589793, 1e6, -0x1p20, 0x1.8p20, 1e22, 0x1p1023, 0x1.fffffffffffffp1023, INFINITY, -INFINITY, NAN, -NAN };
	static const float floats[] = { 0.0f, -0.0f, 0x1p-149f, 0x1p-126f, 0x1p-31f, -0x1p-30f, 0x1p-13f, 0x1p-12f, 0x1p-7f, 0x1p-6f, 0.5f, 1.0f, -1.0f, 2.0f, 3.0f, -4.0f, 5.0f, 3.14159265f, 1e6f, 1e22f, 0x1p127f, 0x1.3170fp+63f, -0x1.2b9622p+67f, 0x1.33333p+13f, INFINITY, -INFINITY, NAN };
	int saved = fegetround();

	for (size_t m = 0; m < mode_count; ++m) {
		CHECK(fesetround(modes[m]) == 0);
		for (size_t i = 0; i < sizeof(doubles) / sizeof(doubles[0]); ++i)
			CHECK(same_double(doubles[i]));
		for (size_t i = 0; i < sizeof(floats) / sizeof(floats[0]); ++i)
			CHECK(same_float(floats[i]));

		/* Near n pi/2, where the reduction cancels most, and their neighbours */
		for (int n = 1; n <= 40; ++n) {
			double d = n * 1.5707963267948966;
			float f = (float)d;

			CHECK(same_double(d) && same_double(nextafter(d, 0.0)) && same_double(nextafter(d, 1e300)) && same_double(-d));
			CHECK(same_float(f) && same_float(nextafterf(f, 0.0f)) && same_float(nextafterf(f, 1e30f)) && same_float(-f));
		}

		/* A sweep of magnitudes */
		for (int e = -40; e <= 100; ++e) {
			for (int k = 0; k < 7; ++k) {
				double d = ldexp(1.0 + k / 7.0, e);
				float f = ldexpf(1.0f + (float)k / 7.0f, e);

				CHECK(same_double(d) && same_double(-d));
				CHECK(same_float(f) && same_float(-f));
			}
		}
	}
	fesetround(saved);
}

static void check_clocks(void)
{
	struct timespec before;
	struct timespec after;
	struct timespec ts;

	/* CLOCK_REALTIME is TIME_UTC */
	CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0 && timespec_get(&after, TIME_UTC) == TIME_UTC);
	CHECK(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000 && nanoseconds(&after) >= nanoseconds(&ts) && nanoseconds(&after) - nanoseconds(&ts) < 1000000000LL);
	CHECK(clock_getres(CLOCK_REALTIME, &ts) == 0 && nanoseconds(&ts) > 0 && nanoseconds(&ts) <= 1000000000LL);
	CHECK(clock_getres(CLOCK_MONOTONIC, &ts) == 0 && nanoseconds(&ts) > 0);
	CHECK(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0 && clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) == 0);

	/* An unknown clock */
	errno = 0;
	CHECK(clock_gettime(99, &ts) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(clock_getres(-1, &ts) == -1 && errno == EINVAL);

	/* nanosleep sleeps at least the time asked, on the monotonic clock */
	CHECK(clock_gettime(CLOCK_MONOTONIC, &before) == 0);
	CHECK(nanosleep(&(struct timespec){ .tv_nsec = 2000000 }, nullptr) == 0);
	CHECK(clock_gettime(CLOCK_MONOTONIC, &after) == 0 && nanoseconds(&after) - nanoseconds(&before) >= 2000000);
	CHECK(nanosleep(&(struct timespec){ 0 }, &ts) == 0);

	/* tv_nsec outside [0, 1e9) */
	errno = 0;
	CHECK(nanosleep(&(struct timespec){ .tv_nsec = 1000000000 }, nullptr) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(nanosleep(&(struct timespec){ .tv_nsec = -1 }, nullptr) == -1 && errno == EINVAL);

	CHECK(sched_yield() == 0);
}

int main(void)
{
	check_descriptors();
	check_file_status();
	check_streams();
	regression_fdopen_failure_keeps_descriptor();
	check_strings();
	check_reallocarray();
	check_sincos();
	check_clocks();

	if (failures == 0)
		puts("pass: posix");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
