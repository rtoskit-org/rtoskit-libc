/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * none.c
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <rtos/rtos-port.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "none-test.h"
#include "rtos-none.h"

/*
 * port/none (P1.12, docs/research/threads.md): a program linked with
 * librtos-none.a starts through __rtos_none_start with a static TLS block,
 * prints through __rtos_none_output, allocates from the board's heap, has
 * one thread (thrd_create fails, mutexes and call_once work, timed waits end
 * at once without a clock), and ends through thrd_exit as exit(EXIT_SUCCESS).
 * Here the board is a Linux process; none.run checks the output and status.
 */

/* The board's heap; its end is defined at link time (none.mk) */
char __rtos_none_heap_start[65536];

/* The board's output and exit */
void __rtos_none_output(const void *buffer, size_t count)
{
	none_test_write(buffer, count);
}

[[noreturn]] void __rtos_exit(int status)
{
	none_test_exit(status);
}

/*
 * PRIMASK and cli are privileged; this single-threaded process has no
 * interrupts, so the critical section and the compare-and-swap need none.
 */
unsigned long __rtos_atomic_enter(void)
{
	return 0;
}

void __rtos_atomic_exit(unsigned long)
{
}

bool __rtos_atomic_cas(volatile unsigned int *address, unsigned int expected, unsigned int desired)
{
	if (*address != expected)
		return false;
	*address = desired;
	return true;
}

/* The board's reset code: describe the TLS image (here from the program headers) and start */
[[noreturn]] void none_test_start(long *)
{
	const unsigned char *headers = (const unsigned char *)&__ehdr_start + __ehdr_start.e_phoff;
	struct __libc_tls_image image = { .init = &__ehdr_start, .init_size = 0, .size = 0, .align = 1 };

	for (unsigned int i = 0; i < __ehdr_start.e_phnum; ++i) {
		const struct __linux_phdr *header = (const struct __linux_phdr *)(headers + i * __ehdr_start.e_phentsize);

		if (header->p_type == 7) {
			image = (struct __libc_tls_image){ .init = (const void *)(uintptr_t)header->p_vaddr, .init_size = header->p_filesz, .size = header->p_memsz, .align = header->p_align };
			break;
		}
	}

	__rtos_none_start(&image);
}

static int failures;
static bool destructor_ran;

static void fail(int line, const char *what)
{
	printf("fail: line %d: %s\n", line, what);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

static thread_local int tls_value = 1234;
static thread_local char tls_buffer[64];
static once_flag once = ONCE_FLAG_INIT;
static int once_calls;

static void once_function(void)
{
	++once_calls;
}

static int never_runs(void *)
{
	return 0;
}

static void destructor(void *value)
{
	destructor_ran = value == &tls_value;
}

/* __rtos_none_start runs .preinit_array once, after the TLS is in place (PLAN D104) */
static int preinit_runs;
static int preinit_tls_value;

static void preinit_hook(void)
{
	++preinit_runs;
	preinit_tls_value = tls_value;
}

[[gnu::section(".preinit_array"), gnu::used]] static void (*const preinit_entry)(void) = preinit_hook;

/* Runs inside exit(EXIT_SUCCESS) after main's thrd_exit */
static void report(void)
{
	CHECK(destructor_ran);
	if (failures == 0)
		puts("pass: none");
}

int main(int argc, char **argv)
{
	thrd_t thread;
	mtx_t mtx;
	cnd_t cnd;
	tss_t key;
	struct timespec ts;
	atomic_int counter = 0;
	_Atomic long long wide = 0;
	char *block;

	CHECK(argc == 0 && argv != nullptr && argv[0] == nullptr);
	CHECK(preinit_runs == 1 && preinit_tls_value == 1234);

	/* The static TLS block holds the initial values */
	CHECK(tls_value == 1234 && tls_buffer[0] == 0);
	errno = ERANGE;
	CHECK(errno == ERANGE);

	/* Heap and formatted output */
	block = malloc(4096);
	CHECK(block != nullptr && (uintptr_t)block >= (uintptr_t)__rtos_none_heap_start && (uintptr_t)block < (uintptr_t)__rtos_none_heap_start + sizeof(__rtos_none_heap_start));
	snprintf(block, 4096, "%d %.3f %s", 42, 2.5, "ok");
	CHECK(strcmp(block, "42 2.500 ok") == 0);
	free(block);

	/* One thread */
	CHECK(thrd_create(&thread, never_runs, nullptr) == thrd_error);
	CHECK(thrd_equal(thrd_current(), thrd_current()) && thrd_join(thrd_current(), nullptr) == thrd_error);
	CHECK(thrd_sleep(&(struct timespec){ .tv_nsec = 1000 }, nullptr) < -1);
	thrd_yield();

	/* Mutexes and condition variables work without blocking */
	CHECK(mtx_init(&mtx, mtx_timed | mtx_recursive) == thrd_success && mtx_lock(&mtx) == thrd_success && mtx_trylock(&mtx) == thrd_success);
	CHECK(mtx_unlock(&mtx) == thrd_success && mtx_unlock(&mtx) == thrd_success && mtx_unlock(&mtx) == thrd_error);
	CHECK(mtx_init(&mtx, mtx_plain) == thrd_success && mtx_lock(&mtx) == thrd_success && mtx_trylock(&mtx) == thrd_busy);
	CHECK(cnd_init(&cnd) == thrd_success && cnd_signal(&cnd) == thrd_success);
	ts = (struct timespec){ .tv_sec = 1, .tv_nsec = 0 };
	CHECK(cnd_timedwait(&cnd, &mtx, &ts) == thrd_timedout && mtx_unlock(&mtx) == thrd_success);

	call_once(&once, once_function);
	call_once(&once, once_function);
	CHECK(once_calls == 1);

	CHECK(tss_create(&key, destructor) == thrd_success && tss_set(key, &tls_value) == thrd_success && tss_get(key) == &tls_value);

	/* Atomics, the 8-byte ones through the library */
	CHECK(atomic_fetch_add(&counter, 2) == 0 && atomic_load(&counter) == 2);
	CHECK(atomic_fetch_add(&wide, 0x100000000LL) == 0 && atomic_load(&wide) == 0x100000000LL);

	/* No clock, no files, no environment */
	CHECK(time(nullptr) == (time_t)-1 && timespec_get(&ts, TIME_UTC) == 0 && clock() == (clock_t)-1);
	errno = 0;
	CHECK(fopen("file", "r") == nullptr && errno == ENOSYS);
	CHECK(getenv("PATH") == nullptr);

	/* The last thread's thrd_exit ends the program as exit(EXIT_SUCCESS), after the tss destructor */
	CHECK(atexit(report) == 0);
	thrd_exit(5);
}
