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
#include <rtos/rtos-port.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

int main(void)
{
	check_seam();
	check_futex_base();

	if (failures == 0)
		puts("pass: pthread");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
