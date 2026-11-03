/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * unwind.c
 *
 * Created on: Nov 3, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>

/*
 * Stack capture as std::stacktrace does it (P7.04, PLAN D128, D134):
 * libgcc's _Unwind_Backtrace walks the Arm EHABI tables between
 * __exidx_start and __exidx_end and reports each frame's address, which is
 * symbolized on the host. The test's own frames always unwind. A frame
 * inside the library (qsort calling the comparison) is passed through only
 * when the library is built with LIBC_UNWIND_TABLES=1; without it, the
 * linker marks the library's code EXIDX_CANTUNWIND and the walk stops there
 * cleanly.
 */

#define FRAMES 32

struct trace {
	uintptr_t pc[FRAMES];
	int count;
};

/* The EHABI table the linker script brackets */
extern const char __exidx_start[];
extern const char __exidx_end[];

static int failures;

static void fail(int line, const char *what)
{
	printf("fail: line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

static _Unwind_Reason_Code record(struct _Unwind_Context *context, void *argument)
{
	struct trace *trace = argument;

	if (trace->count == FRAMES)
		return _URC_END_OF_STACK;
	trace->pc[trace->count++] = _Unwind_GetIP(context);
	return _URC_NO_REASON;
}

/* Returns the frames, and the address the capture returns to (the PC in its caller) */
[[gnu::noipa]] static void capture(struct trace *trace, uintptr_t *returns_to)
{
	trace->count = 0;
	*returns_to = (uintptr_t)__builtin_return_address(0) & ~(uintptr_t)1;
	_Unwind_Backtrace(record, trace);
}

static bool contains(const struct trace *trace, uintptr_t pc)
{
	for (int i = 0; i < trace->count; ++i) {
		if ((trace->pc[i] & ~(uintptr_t)1) == pc)
			return true;
	}
	return false;
}

static struct trace own;
static uintptr_t own_level;
static uintptr_t own_main;

[[gnu::noipa]] static void level(void)
{
	own_main = (uintptr_t)__builtin_return_address(0) & ~(uintptr_t)1;
	capture(&own, &own_level);
}

static struct trace through;
static uintptr_t through_compare;
static uintptr_t through_main;

[[gnu::noipa]] static int compare(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	if (through.count == 0)
		capture(&through, &through_compare);
	return (x > y) - (x < y);
}

[[gnu::noipa]] static void sort(void)
{
	static int values[] = { 3, 1, 2 };

	through_main = (uintptr_t)__builtin_return_address(0) & ~(uintptr_t)1;
	qsort(values, sizeof(values) / sizeof(values[0]), sizeof(values[0]), compare);
	CHECK(values[0] == 1 && values[1] == 2 && values[2] == 3);
}

int main(void)
{
	CHECK((uintptr_t)__exidx_start < (uintptr_t)__exidx_end);

	/* The test's frames: capture, level, main */
	level();
	CHECK(own.count >= 3 && contains(&own, own_level) && contains(&own, own_main));

	/* Through qsort: compare's own frame always; sort and main only past a library frame with tables */
	sort();
	CHECK(through.count >= 2 && contains(&through, through_compare));
	if (UNWIND_LIBRARY)
		CHECK(contains(&through, through_main));
	else
		CHECK(!contains(&through, through_main));

	if (failures == 0)
		puts("pass: unwind");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
