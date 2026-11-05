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

/* Held while a detached thread is checked, so its handle stays valid (a detached thread's handle dies with it) */
static pthread_mutex_t hold = PTHREAD_MUTEX_INITIALIZER;

static void *wait_for_hold(void *)
{
	pthread_mutex_lock(&hold);
	pthread_mutex_unlock(&hold);

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
	CHECK(pthread_mutex_lock(&hold) == 0);
	CHECK(pthread_create(&thread, nullptr, wait_for_hold, nullptr) == 0 && pthread_detach(thread) == 0);
	CHECK(pthread_join(thread, nullptr) == EINVAL && pthread_detach(thread) == EINVAL);
	CHECK(pthread_mutex_unlock(&hold) == 0);

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
	CHECK(pthread_mutex_lock(&hold) == 0);
	CHECK(pthread_create(&thread, &attr, wait_for_hold, nullptr) == 0 && pthread_join(thread, nullptr) == EINVAL);
	CHECK(pthread_mutex_unlock(&hold) == 0);
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

/* P10.04: mutexes, condition variables and keys */
static pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t shared_cond = PTHREAD_COND_INITIALIZER;
static unsigned int counter;
static int ready;

static void *count_up(void *)
{
	for (int i = 0; i < 10000; ++i) {
		pthread_mutex_lock(&shared_mutex);
		++counter;
		pthread_mutex_unlock(&shared_mutex);
	}

	return nullptr;
}

static void *try_from_other(void *arg)
{
	return (void *)(intptr_t)pthread_mutex_trylock(arg);
}

static void *unlock_from_other(void *arg)
{
	return (void *)(intptr_t)pthread_mutex_unlock(arg);
}

static void *signal_ready(void *)
{
	struct timespec pause = { .tv_nsec = 5000000 };

	nanosleep(&pause, nullptr);
	pthread_mutex_lock(&shared_mutex);
	ready = 1;
	pthread_cond_signal(&shared_cond);
	pthread_mutex_unlock(&shared_mutex);

	return nullptr;
}

static struct timespec after(clockid_t clock, long milliseconds)
{
	struct timespec ts;

	clock_gettime(clock, &ts);
	ts.tv_nsec += milliseconds * 1000000L;
	ts.tv_sec += ts.tv_nsec / 1000000000L;
	ts.tv_nsec %= 1000000000L;

	return ts;
}

static unsigned int destructor_calls;
static pthread_key_t again_key;

static void count_destructor(void *)
{
	++destructor_calls;
}

static void again_destructor(void *value)
{
	/* Setting a value again asks for another pass, at most PTHREAD_DESTRUCTOR_ITERATIONS in all */
	++destructor_calls;
	pthread_setspecific(again_key, value);
}

static void *set_keys(void *arg)
{
	pthread_key_t *keys = arg;

	pthread_setspecific(keys[0], &keys[0]);
	pthread_setspecific(again_key, &keys[1]);
	return pthread_getspecific(keys[0]);
}

static void check_mutexes(void)
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;
	pthread_t threads[4];
	struct timespec start;
	struct timespec end;
	struct timespec deadline;
	void *value = nullptr;
	int type;

	/* Attributes */
	CHECK(pthread_mutexattr_init(&attr) == 0 && pthread_mutexattr_gettype(&attr, &type) == 0 && type == PTHREAD_MUTEX_DEFAULT);
	CHECK(pthread_mutexattr_settype(&attr, 7) == EINVAL);
	CHECK(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) == ENOTSUP && pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_PROTECT) == ENOTSUP);

	/* Mutual exclusion under contention */
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_create(&threads[i], nullptr, count_up, nullptr) == 0);
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_join(threads[i], nullptr) == 0);
	CHECK(counter == 40000);

	/* Error-checking: relock EDEADLK, a stranger's unlock EPERM, destroy while locked EBUSY */
	CHECK(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0 && pthread_mutex_init(&mutex, &attr) == 0);
	CHECK(pthread_mutex_unlock(&mutex) == EPERM);
	CHECK(pthread_mutex_lock(&mutex) == 0 && pthread_mutex_lock(&mutex) == EDEADLK && pthread_mutex_trylock(&mutex) == EBUSY);
	CHECK(pthread_mutex_destroy(&mutex) == EBUSY);
	CHECK(pthread_create(&threads[0], nullptr, unlock_from_other, &mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EPERM);
	CHECK(pthread_create(&threads[0], nullptr, try_from_other, &mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EBUSY);

	CHECK(pthread_mutex_unlock(&mutex) == 0 && pthread_mutex_destroy(&mutex) == 0);

	/* Timed and clock-selecting locks against a held mutex: a deadline long past, a bad one, a short wait */
	deadline = (struct timespec){ .tv_sec = 1 };

	CHECK(pthread_mutex_init(&mutex, nullptr) == 0 && pthread_mutex_lock(&mutex) == 0);
	CHECK(pthread_mutex_timedlock(&mutex, &deadline) == ETIMEDOUT);
	CHECK(pthread_mutex_timedlock(&mutex, &(struct timespec){ .tv_nsec = 1000000000 }) == EINVAL);
	CHECK(pthread_mutex_clocklock(&mutex, 42, &deadline) == EINVAL);
	clock_gettime(CLOCK_MONOTONIC, &start);
	deadline = after(CLOCK_MONOTONIC, 20);
	CHECK(pthread_mutex_clocklock(&mutex, CLOCK_MONOTONIC, &deadline) == ETIMEDOUT);
	clock_gettime(CLOCK_MONOTONIC, &end);
	CHECK(nanoseconds(&end) - nanoseconds(&start) >= 20000000);
	CHECK(pthread_mutex_unlock(&mutex) == 0);
	deadline = after(CLOCK_REALTIME, 1000);
	CHECK(pthread_mutex_timedlock(&mutex, &deadline) == 0 && pthread_mutex_unlock(&mutex) == 0);

	/* Recursive: counts, and only the last unlock releases */
	CHECK(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0 && pthread_mutex_init(&mutex, &attr) == 0);
	CHECK(pthread_mutex_lock(&mutex) == 0 && pthread_mutex_lock(&mutex) == 0 && pthread_mutex_trylock(&mutex) == 0);
	CHECK(pthread_mutex_unlock(&mutex) == 0 && pthread_mutex_unlock(&mutex) == 0);
	CHECK(pthread_create(&threads[0], nullptr, try_from_other, &mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EBUSY);
	CHECK(pthread_mutex_unlock(&mutex) == 0 && pthread_mutex_unlock(&mutex) == EPERM);
	CHECK(pthread_mutexattr_destroy(&attr) == 0);
}

static void check_conditions(void)
{
	pthread_condattr_t attr;
	pthread_cond_t cond;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_t thread;
	struct timespec deadline;
	struct timespec start;
	struct timespec end;
	clockid_t clock;

	/* Attributes: CLOCK_REALTIME by default, CLOCK_MONOTONIC on request, nothing else */
	CHECK(pthread_condattr_init(&attr) == 0 && pthread_condattr_getclock(&attr, &clock) == 0 && clock == CLOCK_REALTIME);
	CHECK(pthread_condattr_setclock(&attr, CLOCK_PROCESS_CPUTIME_ID) == EINVAL && pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) == ENOTSUP);
	CHECK(pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) == 0 && pthread_condattr_getclock(&attr, &clock) == 0 && clock == CLOCK_MONOTONIC);

	/* A signal wakes the waiter, which holds the mutex again */
	CHECK(pthread_create(&thread, nullptr, signal_ready, nullptr) == 0);
	CHECK(pthread_mutex_lock(&shared_mutex) == 0);
	while (!ready)
		CHECK(pthread_cond_wait(&shared_cond, &shared_mutex) == 0);
	CHECK(pthread_mutex_trylock(&shared_mutex) == EBUSY && pthread_mutex_unlock(&shared_mutex) == 0);
	CHECK(pthread_join(thread, nullptr) == 0);

	/* Timed waits: on the condition variable's own clock, and on a chosen one */
	CHECK(pthread_cond_init(&cond, &attr) == 0 && pthread_mutex_lock(&mutex) == 0);
	clock_gettime(CLOCK_MONOTONIC, &start);
	deadline = after(CLOCK_MONOTONIC, 20);
	CHECK(pthread_cond_timedwait(&cond, &mutex, &deadline) == ETIMEDOUT);
	clock_gettime(CLOCK_MONOTONIC, &end);
	CHECK(nanoseconds(&end) - nanoseconds(&start) >= 20000000 && pthread_mutex_trylock(&mutex) == EBUSY);
	deadline = after(CLOCK_REALTIME, 10);
	CHECK(pthread_cond_clockwait(&cond, &mutex, CLOCK_REALTIME, &deadline) == ETIMEDOUT);
	CHECK(pthread_cond_clockwait(&cond, &mutex, 42, &deadline) == EINVAL);
	CHECK(pthread_cond_timedwait(&cond, &mutex, &(struct timespec){ .tv_nsec = -1 }) == EINVAL);
	CHECK(pthread_mutex_unlock(&mutex) == 0);

	/* Waiting without the mutex, and the rest */
	CHECK(pthread_cond_wait(&cond, &mutex) == EPERM);
	CHECK(pthread_cond_signal(&cond) == 0 && pthread_cond_broadcast(&cond) == 0 && pthread_cond_destroy(&cond) == 0 && pthread_condattr_destroy(&attr) == 0);
}

static void check_keys(void)
{
	pthread_key_t keys[PTHREAD_KEYS_MAX + 1];
	pthread_t thread;
	void *value = nullptr;
	unsigned int created = 0;

	/* Every key, then EAGAIN */
	while (created < PTHREAD_KEYS_MAX && pthread_key_create(&keys[created], nullptr) == 0)
		++created;
	CHECK(created == PTHREAD_KEYS_MAX && pthread_key_create(&keys[created], nullptr) == EAGAIN);
	for (unsigned int i = 0; i < created; ++i)
		CHECK(pthread_key_delete(keys[i]) == 0);
	CHECK(pthread_key_delete(keys[0]) == EINVAL && pthread_setspecific(keys[0], &value) == EINVAL && pthread_getspecific(keys[0]) == nullptr);
	CHECK(pthread_key_delete(PTHREAD_KEYS_MAX) == EINVAL);

	/* Values per thread, and a deleted key reads as null */
	CHECK(pthread_key_create(&keys[0], count_destructor) == 0 && pthread_key_create(&again_key, again_destructor) == 0);
	CHECK(pthread_setspecific(keys[0], &value) == 0 && pthread_getspecific(keys[0]) == &value);

	/* Destructors at thread exit: one call for keys[0], PTHREAD_DESTRUCTOR_ITERATIONS for the key that sets itself again */
	destructor_calls = 0;
	CHECK(pthread_create(&thread, nullptr, set_keys, keys) == 0 && pthread_join(thread, &value) == 0 && value == &keys[0]);
	CHECK(destructor_calls == 1 + PTHREAD_DESTRUCTOR_ITERATIONS);
	CHECK(pthread_getspecific(keys[0]) == &value);
	CHECK(pthread_key_delete(keys[0]) == 0 && pthread_getspecific(keys[0]) == nullptr && pthread_key_delete(again_key) == 0);
}

/* P10.05: priority-inheritance mutexes */
static pthread_mutex_t pi_mutex;
static unsigned int pi_counter;

static void *pi_count_up(void *)
{
	for (int i = 0; i < 5000; ++i) {
		pthread_mutex_lock(&pi_mutex);
		++pi_counter;
		pthread_mutex_unlock(&pi_mutex);
	}

	return nullptr;
}

static void *pi_timed(void *arg)
{
	struct timespec deadline = after(*(clockid_t *)arg, 20);

	return (void *)(intptr_t)pthread_mutex_clocklock(&pi_mutex, *(clockid_t *)arg, &deadline);
}

static void check_priority_inheritance(void)
{
	pthread_mutexattr_t attr;
	pthread_mutex_t mutex;
	pthread_t threads[4];
	clockid_t realtime = CLOCK_REALTIME;
	clockid_t monotonic = CLOCK_MONOTONIC;
	void *value = nullptr;
	int protocol;

	CHECK(pthread_mutexattr_init(&attr) == 0 && pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT) == 0);
	CHECK(pthread_mutexattr_getprotocol(&attr, &protocol) == 0 && protocol == PTHREAD_PRIO_INHERIT);
	CHECK(pthread_mutexattr_setprotocol(&attr, 7) == EINVAL);

	/* Mutual exclusion through the port's priority-inheritance lock */
	CHECK(pthread_mutex_init(&pi_mutex, &attr) == 0);
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_create(&threads[i], nullptr, pi_count_up, nullptr) == 0);
	for (int i = 0; i < 4; ++i)
		CHECK(pthread_join(threads[i], nullptr) == 0);
	CHECK(pi_counter == 20000);

	/* Held here: another thread's trylock, timed lock on each clock, and unlock fail */
	CHECK(pthread_mutex_lock(&pi_mutex) == 0);
	CHECK(pthread_create(&threads[0], nullptr, try_from_other, &pi_mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EBUSY);
	CHECK(pthread_create(&threads[0], nullptr, pi_timed, &realtime) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == ETIMEDOUT);
	CHECK(pthread_create(&threads[0], nullptr, pi_timed, &monotonic) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == ETIMEDOUT);
	CHECK(pthread_create(&threads[0], nullptr, unlock_from_other, &pi_mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EPERM);
	CHECK(pthread_mutex_destroy(&pi_mutex) == EBUSY && pthread_mutex_unlock(&pi_mutex) == 0 && pthread_mutex_destroy(&pi_mutex) == 0);

	/* The types still decide the owner's relock */
	CHECK(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0 && pthread_mutex_init(&mutex, &attr) == 0);
	CHECK(pthread_mutex_lock(&mutex) == 0 && pthread_mutex_lock(&mutex) == EDEADLK && pthread_mutex_unlock(&mutex) == 0 && pthread_mutex_unlock(&mutex) == EPERM);
	CHECK(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0 && pthread_mutex_init(&mutex, &attr) == 0);
	CHECK(pthread_mutex_lock(&mutex) == 0 && pthread_mutex_trylock(&mutex) == 0 && pthread_mutex_unlock(&mutex) == 0 && pthread_mutex_unlock(&mutex) == 0);

	/* A condition wait releases and retakes a priority-inheritance mutex */
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	struct timespec deadline = after(CLOCK_REALTIME, 10);

	CHECK(pthread_mutex_lock(&mutex) == 0 && pthread_cond_timedwait(&cond, &mutex, &deadline) == ETIMEDOUT);
	CHECK(pthread_create(&threads[0], nullptr, try_from_other, &mutex) == 0 && pthread_join(threads[0], &value) == 0 && (intptr_t)value == EBUSY);
	CHECK(pthread_mutex_unlock(&mutex) == 0 && pthread_mutexattr_destroy(&attr) == 0);
}

int main(void)
{
	check_seam();
	check_futex_base();
	check_threads();
	check_mutexes();
	check_conditions();
	check_keys();
	check_priority_inheritance();

	if (failures == 0)
		puts("pass: pthread");
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
