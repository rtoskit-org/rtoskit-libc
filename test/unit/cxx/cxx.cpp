/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cxx.cpp
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The C library under C++ (PLAN D120, D121; docs/research/cxx-runtime.md):
 * static constructors and destructors, destructors interleaved with atexit
 * handlers in reverse order of registration, more registrations than one
 * block holds, __cxa_finalize, thread_local destructors on the main thread
 * and on a <threads.h> thread, new and delete, exceptions across frames and
 * through destructors, and dynamic_cast. The C++ runtime is the toolchain's
 * libsupc++; every C library function comes from libc.a.
 */
#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbit.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <tgmath.h>
#include <threads.h>
#include <time.h>
#include <rtos/rtos-port.h>

#include "cxx.h"

static char trace[64];
static unsigned int traced;
static int failures;

static void note(char c)
{
	if (traced < sizeof(trace) - 1)
		trace[traced++] = c;
}

static void check(bool ok, const char *what)
{
	if (!ok) {
		printf("FAIL %s (trace \"%s\")\n", what, trace);
		++failures;
	}
}

/* Notes its letter in upper case when constructed and in lower case when destroyed */
struct tracer {
	char letter;

	explicit tracer(char c)
			: letter(c)
	{
		note((char)toupper(c));
	}
	~tracer()
	{
		note(letter);
	}
};

/* More registrations than a block of __LIBC_CXA_BLOCK_ENTRIES (32): they must run newest first */
static constexpr unsigned int many = 70;
static unsigned int many_next = many;

static void many_destructor(void *arg)
{
	check((uintptr_t)arg == many_next - 1, "registrations beyond one block run newest first");
	--many_next;
}

/* Runs last: registered before any static object is constructed */
static void final_check(void)
{
	check(strcmp(trace, "ABTDtdxqba") == 0, "exit order");
	check(many_next == 0, "every registration ran");
	printf("%s\n", failures == 0 ? "cxx: ok" : "cxx: failed");
	_Exit(failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

static struct first {
	first()
	{
		atexit(final_check);
	}
} first_object;

static tracer a('a');
static tracer b('b');

static void exit_handler(void)
{
	note('x');
}

static void queued(void *)
{
	note('q');
}

static thread_local tracer main_thread_object('t');

static int thread_body(void *)
{
	static thread_local tracer thread_object('u');

	return thread_object.letter == 'u' ? 0 : 1;
}

struct shape {
	virtual ~shape() = default;
	virtual int sides() const = 0;
};

struct square : shape {
	int sides() const override
	{
		return 4;
	}
};

struct circle : shape {
	int sides() const override
	{
		return 0;
	}
};

struct failure {
	int code;
};

static int unwound;

struct guard {
	~guard()
	{
		++unwound;
	}
};

[[gnu::noinline]] static void thrower(int depth)
{
	guard g;

	if (depth == 0)
		throw failure{ 42 };
	thrower(depth - 1);
}

static void test_exceptions(void)
{
	int caught = 0;

	try {
		thrower(3);
	} catch (const failure &e) {
		caught = e.code;
	}
	check(caught == 42 && unwound == 4, "exception across frames, destructors run");

	caught = 0;
	try {
		try {
			throw 7;
		} catch (int) {
			throw;
		}
	} catch (int value) {
		caught = value;
	}
	check(caught == 7, "rethrow");
}

static void test_dynamic(void)
{
	shape *one = new square;
	shape *other = new circle;

	check(dynamic_cast<square *>(one) != nullptr && dynamic_cast<square *>(other) == nullptr, "dynamic_cast");
	check(one->sides() == 4 && other->sides() == 0, "virtual calls");
	delete one;
	delete other;

	int *array = new int[100];

	for (int i = 0; i < 100; ++i)
		array[i] = i;
	check(array[99] == 99, "new[]");
	delete[] array;
}

static void test_finalize(void)
{
	static char image;
	static int ran;

	check(__cxa_atexit([](void *arg) { ++*(int *)arg; }, &ran, &image) == 0, "__cxa_atexit");
	__cxa_finalize(&image);
	__cxa_finalize(&image);
	check(ran == 1, "__cxa_finalize runs an image's destructors once");
}

int main(void)
{
	thrd_t thread;
	int result = -1;

	check(strcmp(trace, "AB") == 0, "static constructors in order");
	check(headers17() == 0, "the headers as C++17");

	test_exceptions();
	test_dynamic();
	test_finalize();

	check(thrd_create(&thread, thread_body, nullptr) == thrd_success && thrd_join(thread, &result) == thrd_success && result == 0, "thread");
	check(strcmp(trace, "ABUu") == 0, "thread_local destructor at thread exit");
	traced = 2;
	trace[2] = '\0';

	/* Exit runs the thread_local t, then newest first: d, x, the many, q, b, a, and final_check */
	check(main_thread_object.letter == 't', "main thread_local");
	__cxa_atexit(queued, nullptr, nullptr);
	for (unsigned int i = 0; i < many; ++i)
		__cxa_atexit(many_destructor, (void *)(uintptr_t)i, nullptr);
	atexit(exit_handler);

	static tracer d('d');

	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
