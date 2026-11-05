/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * pthread.c
 *
 * Created on: Nov 5, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <rtos/rtos-port.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * POSIX threads (phase 10, docs/research/pthreads.md) on the Linux test
 * port. The seam's thread attributes, exit value, scheduling and futex time
 * base (P10.02, PLAN D148-D150) come first; each later step adds its own.
 */
static int failures;

static void check(int line, bool ok, const char *what)
{
	if (!ok) {
		printf("fail: line %d: %s\n", line, what);
		++failures;
	}
}

#define CHECK(condition) check(__LINE__, (condition), #condition)

static long long nanoseconds(const struct timespec *ts)
{
	return ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

/* What a created thread saw of itself */
struct seam_report {
	int policy;
	int priority;
	uintptr_t local;
};

static void *seam_thread(void *arg)
{
	struct seam_report *report = arg;
	int local = 0;

	__rtos_thread_get_sched(__rtos_thread_self(), &report->policy, &report->priority);
	report->local = (uintptr_t)&local;

	return report;
}

static void check_seam(void)
{
	static unsigned char stack[64 << 10] __attribute__((aligned(16)));
	struct seam_report report = { 0 };
	struct __rtos_thread *thread;
	struct __rtos_thread_attr attr = { .policy = SCHED_FIFO, .priority = 10 };
	int policy;
	int priority;
	int min;
	int max;
	void *result = nullptr;

	/* D149: a range per policy, and EINVAL for one the port does not have */
	CHECK(__rtos_sched_priority_range(SCHED_FIFO, &min, &max) == 0 && min == 1 && max == 99);
	CHECK(__rtos_sched_priority_range(SCHED_RR, &min, &max) == 0 && min == 1 && max == 99);
	CHECK(__rtos_sched_priority_range(SCHED_OTHER, &min, &max) == 0 && min == 0 && max == 0);
	CHECK(__rtos_sched_priority_range(42, &min, &max) == -EINVAL);

	/* The initial thread, and the checks on a new setting */
	CHECK(__rtos_thread_get_sched(__rtos_thread_self(), &policy, &priority) == 0 && policy == SCHED_OTHER && priority == 0);
	CHECK(__rtos_thread_set_sched(__rtos_thread_self(), SCHED_FIFO, 100) == -EINVAL);
	CHECK(__rtos_thread_set_sched(__rtos_thread_self(), SCHED_OTHER, 1) == -EINVAL);

	/* Explicit scheduling, and the void * exit value */
	CHECK(__rtos_thread_create(&thread, &attr, seam_thread, &report) == 0);
	CHECK(__rtos_thread_join(thread, &result) == 0 && result == &report);
	CHECK(report.policy == SCHED_FIFO && report.priority == 10);

	/* A null attribute, and an inheriting one, take the creator's setting */
	CHECK(__rtos_thread_set_sched(__rtos_thread_self(), SCHED_RR, 20) == 0);
	CHECK(__rtos_thread_create(&thread, nullptr, seam_thread, &report) == 0 && __rtos_thread_join(thread, nullptr) == 0);
	CHECK(report.policy == SCHED_RR && report.priority == 20);
	attr = (struct __rtos_thread_attr){ .inherit = true, .policy = SCHED_FIFO, .priority = 5 };
	CHECK(__rtos_thread_create(&thread, &attr, seam_thread, &report) == 0 && __rtos_thread_join(thread, nullptr) == 0);
	CHECK(report.policy == SCHED_RR && report.priority == 20);
	CHECK(__rtos_thread_set_sched(__rtos_thread_self(), SCHED_OTHER, 0) == 0);

	/* An explicit priority outside the range is refused */
	attr = (struct __rtos_thread_attr){ .policy = SCHED_FIFO, .priority = 0 };
	CHECK(__rtos_thread_create(&thread, &attr, seam_thread, &report) == -EINVAL);

	/* The caller's stack */
	attr = (struct __rtos_thread_attr){ .stack = stack, .stack_size = sizeof(stack), .inherit = true };
	CHECK(__rtos_thread_create(&thread, &attr, seam_thread, &report) == 0 && __rtos_thread_join(thread, nullptr) == 0);
	CHECK(report.local >= (uintptr_t)stack && report.local < (uintptr_t)stack + sizeof(stack));
}

static void check_futex_base(void)
{
	unsigned int word = 0;
	struct timespec start;
	struct timespec end;
	struct timespec deadline;

	/* D150: a deadline on either time base; anything else is EINVAL */
	CHECK(__rtos_futex_wait(&word, 0, 42, nullptr) == -EINVAL);
	CHECK(__rtos_futex_wait(&word, 0, TIME_MONOTONIC, &(struct timespec){ .tv_nsec = 1000000000 }) == -EINVAL);
	CHECK(__rtos_futex_wait(&word, 1, TIME_MONOTONIC, nullptr) == -EAGAIN);
	CHECK(__rtos_futex_wait(&word, 0, TIME_MONOTONIC, &(struct timespec){ .tv_sec = 0 }) == -ETIMEDOUT);

	CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
	deadline = (struct timespec){ .tv_sec = start.tv_sec, .tv_nsec = start.tv_nsec + 20000000 };
	if (deadline.tv_nsec >= 1000000000) {
		deadline.tv_nsec -= 1000000000;
		++deadline.tv_sec;
	}
	CHECK(__rtos_futex_wait(&word, 0, TIME_MONOTONIC, &deadline) == -ETIMEDOUT);
	CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0 && nanoseconds(&end) >= nanoseconds(&deadline));

	CHECK(clock_gettime(CLOCK_REALTIME, &start) == 0);
	deadline = (struct timespec){ .tv_sec = start.tv_sec, .tv_nsec = start.tv_nsec };
	CHECK(__rtos_futex_wait(&word, 0, TIME_UTC, &deadline) == -ETIMEDOUT);
}

/* P10.03: the thread functions, attributes, scheduling and pthread_once */
static void *return_arg(void *arg)
{
	return arg;
}

static void *exit_arg(void *arg)
{
	pthread_exit(arg);
}

static void *report_sched(void *arg)
{
	struct sched_param param;
	int *policy = arg;

	pthread_getschedparam(pthread_self(), policy, &param);
	policy[1] = param.sched_priority;

	return nullptr;
}

static void *report_stack(void *arg)
{
	int local = 0;

	*(uintptr_t *)arg = (uintptr_t)&local;

	return nullptr;
}

static unsigned int once_runs;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static void once_body(void)
{
	struct timespec pause = { .tv_nsec = 2000000 };

	/* Long enough for the other threads to arrive and wait */
	nanosleep(&pause, nullptr);
	__atomic_fetch_add(&once_runs, 1, __ATOMIC_RELAXED);
}

static void *once_thread(void *)
{
	return (void *)(intptr_t)pthread_once(&once, once_body);
}

static void check_threads(void)
{
	static unsigned char stack[64 << 10] __attribute__((aligned(16)));
	pthread_t thread;
	pthread_t threads[4];
	pthread_attr_t attr;
	struct sched_param param;
	void *value = nullptr;
	int report[2];
	int detach;
	int inherit;
	int policy;
	int scope;
	size_t size;
	void *address;
	uintptr_t local = 0;

	/* create, join, the value of a return and of pthread_exit */
	CHECK(pthread_create(&thread, nullptr, return_arg, &value) == 0 && pthread_join(thread, &value) == 0 && value == &value);
	CHECK(pthread_create(&thread, nullptr, exit_arg, &once) == 0 && pthread_join(thread, &value) == 0 && value == &once);
	CHECK(pthread_equal(pthread_self(), pthread_self()) && !pthread_equal(pthread_self(), thread));

	/* Joining oneself, a detached thread, and detaching twice */
	CHECK(pthread_join(pthread_self(), nullptr) == EDEADLK);
	CHECK(pthread_create(&thread, nullptr, return_arg, nullptr) == 0 && pthread_detach(thread) == 0);
	CHECK(pthread_join(thread, nullptr) == EINVAL && pthread_detach(thread) == EINVAL);

	/* Attributes: defaults, and the values each setter refuses */
	CHECK(pthread_attr_init(&attr) == 0);
	CHECK(pthread_attr_getdetachstate(&attr, &detach) == 0 && detach == PTHREAD_CREATE_JOINABLE);
	CHECK(pthread_attr_getinheritsched(&attr, &inherit) == 0 && inherit == PTHREAD_INHERIT_SCHED);
	CHECK(pthread_attr_getscope(&attr, &scope) == 0 && scope == PTHREAD_SCOPE_SYSTEM);
	CHECK(pthread_attr_getschedpolicy(&attr, &policy) == 0 && policy == SCHED_OTHER);
	CHECK(pthread_attr_setdetachstate(&attr, 7) == EINVAL && pthread_attr_setinheritsched(&attr, 7) == EINVAL);
	CHECK(pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS) == ENOTSUP && pthread_attr_setscope(&attr, 7) == EINVAL);
	CHECK(pthread_attr_setschedpolicy(&attr, 42) == EINVAL);
	CHECK(pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN - 1) == EINVAL && pthread_attr_setstack(&attr, stack, PTHREAD_STACK_MIN - 1) == EINVAL);
	CHECK(pthread_attr_setstacksize(&attr, 128 << 10) == 0 && pthread_attr_getstacksize(&attr, &size) == 0 && size == (128 << 10));

	/* A detached thread from its attribute */
	CHECK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);
	CHECK(pthread_create(&thread, &attr, return_arg, nullptr) == 0 && pthread_join(thread, nullptr) == EINVAL);
	CHECK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0);

	/* Explicit scheduling, a priority outside the range, and inheritance */
	param.sched_priority = 30;
	CHECK(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0 && pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0 && pthread_attr_setschedparam(&attr, &param) == 0);
	CHECK(pthread_create(&thread, &attr, report_sched, report) == 0 && pthread_join(thread, nullptr) == 0);
	CHECK(report[0] == SCHED_RR && report[1] == 30);
	param.sched_priority = 100;
	CHECK(pthread_attr_setschedparam(&attr, &param) == 0 && pthread_create(&thread, &attr, report_sched, report) == EINVAL);
	param.sched_priority = 40;
	CHECK(pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) == 0);
	CHECK(pthread_create(&thread, nullptr, report_sched, report) == 0 && pthread_join(thread, nullptr) == 0);
	CHECK(report[0] == SCHED_FIFO && report[1] == 40);
	CHECK(pthread_setschedprio(pthread_self(), 41) == 0 && pthread_getschedparam(pthread_self(), &policy, &param) == 0 && policy == SCHED_FIFO && param.sched_priority == 41);
	CHECK(pthread_setschedprio(pthread_self(), 0) == EINVAL);
	param.sched_priority = 0;
	CHECK(pthread_setschedparam(pthread_self(), SCHED_OTHER, &param) == 0);
	CHECK(pthread_setschedparam(pthread_self(), 42, &param) == EINVAL);

	/* sched_get_priority_max/min */
	CHECK(sched_get_priority_min(SCHED_FIFO) == 1 && sched_get_priority_max(SCHED_FIFO) == 99 && sched_get_priority_max(SCHED_OTHER) == 0);
	errno = 0;
	CHECK(sched_get_priority_max(42) == -1 && errno == EINVAL);

	/* The caller's stack */
	CHECK(pthread_attr_init(&attr) == 0 && pthread_attr_setstack(&attr, stack, sizeof(stack)) == 0);
	CHECK(pthread_attr_getstack(&attr, &address, &size) == 0 && address == stack && size == sizeof(stack));
	CHECK(pthread_create(&thread, &attr, report_stack, &local) == 0 && pthread_join(thread, nullptr) == 0);
	CHECK(local >= (uintptr_t)stack && local < (uintptr_t)stack + sizeof(stack));
	CHECK(pthread_attr_destroy(&attr) == 0);

	/* pthread_once raced by four threads runs its routine once */
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_create(&threads[i], nullptr, once_thread, nullptr) == 0);
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_join(threads[i], &value) == 0 && value == nullptr);
	CHECK(once_runs == 1 && pthread_once(&once, once_body) == 0 && once_runs == 1);

	/* The option macros (<unistd.h>) and limits (<limits.h>) */
	CHECK(_POSIX_THREADS > 0 && _POSIX_TIMEOUTS > 0 && _POSIX_THREAD_PRIORITY_SCHEDULING > 0);
	CHECK(PTHREAD_KEYS_MAX == 32 && PTHREAD_DESTRUCTOR_ITERATIONS == 4 && PTHREAD_STACK_MIN == 512);
}

int main(void)
{
	check_seam();
	check_futex_base();
	check_threads();

	if (failures == 0)
		puts("pass: pthread");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
