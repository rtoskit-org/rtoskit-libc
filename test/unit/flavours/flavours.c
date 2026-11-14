/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * flavours.c
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * The printf and scanf flavours (P2.03, PLAN D53). One source, linked three
 * times by flavours.mk: against libc.a (full), libc-nofloat.a and
 * libc-minimal-printf.a. The first argument names the archive, and the
 * checks expect what that flavour keeps and what it refuses.
 */

enum flavour {
	FULL,
	NOFLOAT,
	MINIMAL,
};

static int failures;

static void fail(int line, const char *what)
{
	char message[160];

	snprintf(message, sizeof(message), "fail: line %d: %s\n", line, what);
	fputs(message, stdout);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

/* snprintf into buffer; returns its result, with errno cleared first */
static int format(char *buffer, size_t size, const char *spec, ...)
{
	va_list ap;
	int status;

	errno = 0;
	va_start(ap, spec);
	status = vsnprintf(buffer, size, spec, ap);
	va_end(ap);

	return status;
}

/* What every flavour keeps: integers up to long, characters, strings, pointers, flags, width, precision */
static void check_common(void)
{
	char buffer[64];

	CHECK(format(buffer, sizeof(buffer), "%d|%i|%u|%x|%X|%o|%b", -42, 7, 42u, 0xbeefu, 0xbeefu, 8u, 5u) > 0 && strcmp(buffer, "-42|7|42|beef|BEEF|10|101") == 0);
	CHECK(format(buffer, sizeof(buffer), "%ld|%lu|%hhd|%hu|%zu|%td", LONG_MIN, ULONG_MAX, -1, 65535, (size_t)3, (ptrdiff_t)-4) > 0);
	CHECK(strtol(buffer, nullptr, 10) == LONG_MIN);
	CHECK(format(buffer, sizeof(buffer), "[%-6d|%06d|%+.3d|% d|%#x|%#o]", 12, -12, 5, 3, 255u, 8u) > 0 && strcmp(buffer, "[12    |-00012|+005| 3|0xff|010]") == 0);
	CHECK(format(buffer, sizeof(buffer), "%c%s%.2s%5s", 'a', "bc", "def", "g") > 0 && strcmp(buffer, "abcde    g") == 0);
	CHECK(format(buffer, sizeof(buffer), "%p", (void *)0x1234) > 0 && strcmp(buffer, "0x1234") == 0);
	CHECK(format(buffer, sizeof(buffer), "%w32d|%w16u|%wf8d", INT32_MIN, (uint16_t)65535, (int_fast8_t)-8) > 0 && strcmp(buffer, "-2147483648|65535|-8") == 0);
}

/* 64-bit lengths: refused by the minimal flavour where they are wider than long */
static void check_wide(enum flavour flavour)
{
	char buffer[64];
	bool wider = sizeof(long long) > sizeof(long);
	int status = format(buffer, sizeof(buffer), "%lld|%jx|%w64u", LLONG_MIN, (uintmax_t)UINT64_MAX, (uint64_t)1 << 40);

	if (flavour == MINIMAL && wider) {
		CHECK(status < 0 && errno == EINVAL);
	} else {
		CHECK(status > 0 && strcmp(buffer, "-9223372036854775808|ffffffffffffffff|1099511627776") == 0);
	}
}

/* %n: refused by the minimal flavour */
static void check_count(enum flavour flavour)
{
	char buffer[16];
	int count = -1;
	int status = format(buffer, sizeof(buffer), "abc%n", &count);

	if (flavour == MINIMAL)
		CHECK(status < 0 && errno == EINVAL && count == -1);
	else
		CHECK(status == 3 && count == 3);
}

/* Floating point: printf refuses it and scanf fails to match it, except in the full flavour; strtod always converts */
static void check_float(enum flavour flavour)
{
	char buffer[32];
	float value = 0.0f;
	int number = 0;
	int status = format(buffer, sizeof(buffer), "x=%d %.2f", 4, 1.5);

	if (flavour == FULL) {
		CHECK(status == 8 && strcmp(buffer, "x=4 1.50") == 0);
	} else {
		/* The output before the failing conversion was written; the result is the failure */
		CHECK(status < 0 && errno == EINVAL && strncmp(buffer, "x=4 ", 4) == 0);
	}

	errno = 0;
	status = sscanf("12 3.25", "%d %f", &number, &value);
	if (flavour == FULL)
		CHECK(status == 2 && number == 12 && value == 3.25f);
	else
		CHECK(status == 1 && number == 12 && value == 0.0f && errno == EINVAL);

	CHECK(strtod("2.5", nullptr) == 2.5);
}

/* scanf: %n and the stores wider than long are refused by the minimal flavour, as printf refuses them */
static void check_scan(enum flavour flavour)
{
	bool wider = sizeof(long long) > sizeof(long);
	long long big = 0;
	int number = 0;
	int count = -1;
	int status;

	errno = 0;
	status = sscanf("7 abc", "%d%n", &number, &count);
	if (flavour == MINIMAL)
		CHECK(status == 1 && number == 7 && count == -1 && errno == EINVAL);
	else
		CHECK(status == 1 && number == 7 && count == 1);

	errno = 0;
	status = sscanf("-123456789012", "%lld", &big);
	if (flavour == MINIMAL && wider)
		CHECK(status == 0 && big == 0 && errno == EINVAL);
	else
		CHECK(status == 1 && big == -123456789012LL);
}

/* The headers every build type shares (PLAN D162, flavours.run): libm reports errors by exceptions only, fast included (D45, D163) */
static void check_headers(void)
{
	CHECK(math_errhandling == MATH_ERREXCEPT);
}

int main(int argc, char **argv)
{
	enum flavour flavour = FULL;

	if (argc > 1 && strcmp(argv[1], "nofloat") == 0)
		flavour = NOFLOAT;
	else if (argc > 1 && strcmp(argv[1], "minimal") == 0)
		flavour = MINIMAL;

	check_headers();
	check_common();
	check_wide(flavour);
	check_count(flavour);
	check_float(flavour);
	check_scan(flavour);

	if (failures == 0)
		printf("pass: flavours %s\n", argc > 1 ? argv[1] : "full");

	return failures == 0 ? 0 : 1;
}
