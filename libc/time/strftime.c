/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strftime.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <time.h>

#include "internal/div64.h"
#include "internal/time.h"

/*
 * C23 7.29.3.5 in the "C" locale (docs/research/time-locale.md). The E and O
 * modifiers select the same output. POSIX adds an optional flag ('0' or '+')
 * and minimum width before the conversion: for %C, %F, %G and %Y the '+'
 * flag adds a sign to a non-negative year with more digits than the usual
 * four (two for %C) or a wider field. Plain %Y, %G and %F behave as %+4Y
 * (POSIX), plain %C as %02C (test/unit/time, test/unit/conformance).
 */

struct output {
	char *s;
	size_t max;
	size_t n;
};

/* What a conversion needs besides the fields, read only when a conversion asks */
struct context {
	const struct tm *tm;
	struct __libc_tz tz;
	bool have_tz;
};

static const char *const day_names[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char *const month_names[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

static void put(struct output *out, char c)
{
	if (out->n < out->max)
		out->s[out->n] = c;
	++out->n;
}

static void put_padding(struct output *out, char pad, int count)
{
	while (count-- > 0)
		put(out, pad);
}

/* text, at most limit characters, right-aligned in width */
static void put_text(struct output *out, const char *text, size_t limit, int width)
{
	size_t length = 0;

	while (length < limit && text[length] != '\0')
		++length;
	put_padding(out, ' ', width - (int)length);
	for (size_t i = 0; i < length; ++i)
		put(out, text[i]);
}

/* The digits of value, least significant first, without the 64-bit division helpers (PLAN D54) */
static int render(char *digits, unsigned long long value)
{
	int n = 0;

	do {
		uint32_t digit;

		value = __libc_udivmod64(value, 10, &digit);
		digits[n++] = (char)('0' + digit);
	} while (value != 0);

	return n;
}

/* The year's last two digits, 00 to 99 also for negative years */
static int year_in_century(long long year)
{
	uint32_t rest;

	__libc_floor_divmod64(year, 100, &rest);
	return (int)rest;
}

/* %C: the year divided by 100, truncated toward zero as C division is */
static long long century(long long year)
{
	uint32_t rest;
	long long magnitude = (long long)__libc_udivmod64(year < 0 ? -(unsigned long long)year : (unsigned long long)year, 100, &rest);

	return year < 0 ? -magnitude : magnitude;
}

/* A field of at least digits digits padded with pad, widened to width by the flag's zeros or by spaces */
static void put_number(struct output *out, long long value, int digits, char pad, char flag, int width)
{
	char text[24];
	unsigned long long magnitude = value < 0 ? -(unsigned long long)value : (unsigned long long)value;
	int n = render(text, magnitude);
	int length = (n > digits ? n : digits) + (value < 0);

	if (flag != '\0' && width > length) {
		pad = '0';
		digits = width - (value < 0);
		length = width;
	}
	put_padding(out, ' ', width - length);
	if (value < 0)
		put(out, '-');
	put_padding(out, pad, digits - n);
	while (n > 0)
		put(out, text[--n]);
}

/* A year-like field under the POSIX rules above; the width includes the sign */
static void put_year(struct output *out, long long value, char flag, int width, int threshold)
{
	char text[24];
	unsigned long long magnitude = value < 0 ? -(unsigned long long)value : (unsigned long long)value;
	int n = render(text, magnitude);
	char sign = value < 0 ? '-' : flag == '+' && (n > threshold || width > threshold) ? '+' :
																						'\0';

	if (sign != '\0') {
		put(out, sign);
		--width;
	}
	put_padding(out, '0', width - n);
	while (n > 0)
		put(out, text[--n]);
}

static long long full_year(const struct tm *tm)
{
	return (long long)tm->tm_year + 1900;
}

/* 1 for days in a week that starts on first_day (0 Sunday, 1 Monday) */
static int week_number(const struct tm *tm, int first_day)
{
	int weekday = (tm->tm_wday + 7 - first_day) % 7;

	return (tm->tm_yday + 7 - weekday) / 7;
}

/* The number of ISO 8601 weeks in a year whose January 1 falls on ISO weekday jan1 (0 Monday) */
static int iso_weeks(long long year, int jan1)
{
	return jan1 == 3 || (jan1 == 2 && __libc_is_leap(year)) ? 53 : 52;
}

/* C23 7.29.3.5p7: the ISO 8601 week-based year and week number of tm */
static long long iso_week(const struct tm *tm, int *week)
{
	long long year = full_year(tm);
	int weekday = (tm->tm_wday + 6) % 7;
	int jan1 = ((weekday - tm->tm_yday) % 7 + 7) % 7;
	int number = (tm->tm_yday - weekday + 10) / 7;

	if (number < 1) {
		/* The last week of the previous year */
		int previous_jan1 = ((jan1 - (__libc_is_leap(year - 1) ? 366 : 365)) % 7 + 7) % 7;

		--year;
		number = iso_weeks(year, previous_jan1);
	} else if (number > iso_weeks(year, jan1)) {
		++year;
		number = 1;
	}

	*week = number;
	return year;
}

static const struct __libc_tz *zone(struct context *context)
{
	if (!context->have_tz) {
		__libc_tz_get(&context->tz);
		context->have_tz = true;
	}

	return &context->tz;
}

static bool daylight(struct context *context)
{
	return context->tm->tm_isdst > 0 && zone(context)->has_dst;
}

static void format(struct output *out, const char *fmt, struct context *context);

static void convert(struct output *out, char conversion, char flag, int width, struct context *context)
{
	const struct tm *tm = context->tm;
	unsigned int wday = tm->tm_wday;
	unsigned int mon = tm->tm_mon;
	long long year;
	int week;

	switch (conversion) {
		case 'a':
			put_text(out, wday < 7 ? day_names[wday] : "?", 3, width);
			break;
		case 'A':
			put_text(out, wday < 7 ? day_names[wday] : "?", 16, width);
			break;
		case 'b':
		case 'h':
			put_text(out, mon < 12 ? month_names[mon] : "?", 3, width);
			break;
		case 'B':
			put_text(out, mon < 12 ? month_names[mon] : "?", 16, width);
			break;
		case 'c':
			format(out, "%a %b %e %T %Y", context);
			break;
		case 'C':
			put_year(out, century(full_year(tm)), flag == '\0' ? '0' : flag, width < 0 ? 2 : width, 2);
			break;
		case 'd':
			put_number(out, tm->tm_mday, 2, '0', flag, width);
			break;
		case 'D':
			format(out, "%m/%d/%y", context);
			break;
		case 'e':
			put_number(out, tm->tm_mday, 2, ' ', flag, width);
			break;
		case 'F':
			/* The year takes what the width leaves after "-mm-dd" */
			put_year(out, full_year(tm), flag == '\0' ? '+' : flag, width < 0 ? 4 : width - 6, 4);
			format(out, "-%m-%d", context);
			break;
		case 'g':
			year = iso_week(tm, &week);
			put_number(out, year_in_century(year), 2, '0', flag, width);
			break;
		case 'G':
			put_year(out, iso_week(tm, &week), flag == '\0' ? '+' : flag, width < 0 ? 4 : width, 4);
			break;
		case 'H':
			put_number(out, tm->tm_hour, 2, '0', flag, width);
			break;
		case 'I':
			put_number(out, (tm->tm_hour + 11) % 12 + 1, 2, '0', flag, width);
			break;
		case 'j':
			put_number(out, tm->tm_yday + 1, 3, '0', flag, width);
			break;
		case 'm':
			put_number(out, tm->tm_mon + 1, 2, '0', flag, width);
			break;
		case 'M':
			put_number(out, tm->tm_min, 2, '0', flag, width);
			break;
		case 'n':
			put(out, '\n');
			break;
		case 'p':
			put_text(out, tm->tm_hour < 12 ? "AM" : "PM", 2, width);
			break;
		case 'r':
			format(out, "%I:%M:%S %p", context);
			break;
		case 'R':
			format(out, "%H:%M", context);
			break;
		case 's':
			/* POSIX: seconds since the epoch of the fields read as local time */
			put_number(out, __libc_tm_to_secs(tm) - (daylight(context) ? zone(context)->dst_offset : zone(context)->std_offset), 1, '0', flag, width);
			break;
		case 'S':
			put_number(out, tm->tm_sec, 2, '0', flag, width);
			break;
		case 't':
			put(out, '\t');
			break;
		case 'T':
		case 'X':
			format(out, "%H:%M:%S", context);
			break;
		case 'u':
			put_number(out, (tm->tm_wday + 6) % 7 + 1, 1, '0', flag, width);
			break;
		case 'U':
			put_number(out, week_number(tm, 0), 2, '0', flag, width);
			break;
		case 'V':
			iso_week(tm, &week);
			put_number(out, week, 2, '0', flag, width);
			break;
		case 'w':
			put_number(out, tm->tm_wday, 1, '0', flag, width);
			break;
		case 'W':
			put_number(out, week_number(tm, 1), 2, '0', flag, width);
			break;
		case 'x':
			format(out, "%m/%d/%y", context);
			break;
		case 'y':
			put_number(out, year_in_century(full_year(tm)), 2, '0', flag, width);
			break;
		case 'Y':
			put_year(out, full_year(tm), flag == '\0' ? '+' : flag, width < 0 ? 4 : width, 4);
			break;
		case 'z':
			/* C23 7.29.3.5p6: no characters when the time zone is unknown (tm_isdst negative) */
			if (tm->tm_isdst >= 0) {
				long offset = daylight(context) ? zone(context)->dst_offset : zone(context)->std_offset;
				long minutes = (offset < 0 ? -offset : offset) / 60;

				put(out, offset < 0 ? '-' : '+');
				put_number(out, minutes / 60 * 100 + minutes % 60, 4, '0', '\0', -1);
			}
			break;
		case 'Z':
			if (tm->tm_isdst >= 0)
				put_text(out, daylight(context) ? zone(context)->dst_name : zone(context)->std_name, __LIBC_TZ_NAME_MAX, width);
			break;
		case '%':
			put(out, '%');
			break;
		default:
			/* Undefined behaviour (7.29.3.5p3): copy the conversion specification */
			put(out, '%');
			put(out, conversion);
			break;
	}
}

static void format(struct output *out, const char *fmt, struct context *context)
{
	while (*fmt != '\0') {
		char flag = '\0';
		int width = -1;

		if (*fmt != '%') {
			put(out, *fmt++);
			continue;
		}

		++fmt;
		if (*fmt == '0' || *fmt == '+')
			flag = *fmt++;
		if (*fmt >= '1' && *fmt <= '9') {
			width = 0;
			while (*fmt >= '0' && *fmt <= '9') {
				if (width < 1000)
					width = width * 10 + (*fmt - '0');
				++fmt;
			}
		}
		if (*fmt == 'E' || *fmt == 'O')
			++fmt;
		if (*fmt == '\0') {
			put(out, '%');
			break;
		}

		convert(out, *fmt++, flag, width, context);
	}
}

size_t strftime(char *restrict s, size_t maxsize, const char *restrict format_string, const struct tm *restrict timeptr)
{
	struct output out = { .s = s, .max = maxsize, .n = 0 };
	struct context context = { .tm = timeptr, .have_tz = false };

	format(&out, format_string, &context);

	/* C23 7.29.3.5p8: zero, and indeterminate contents, when the result and its null do not fit */
	if (out.n >= maxsize)
		return 0;
	s[out.n] = '\0';

	return out.n;
}
