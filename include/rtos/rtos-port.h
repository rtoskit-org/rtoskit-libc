/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * rtos-port.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _RTOS_RTOS_PORT_H
#define _RTOS_RTOS_PORT_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/*
 * The RTOS seam (PLAN D4; process and I/O from P1.01, heap and locks from P1.04,
 * environment from P1.05, files and the stream locks from P1.09, clocks from
 * P1.11, threads, futexes and the atomics critical section from P1.12).
 *
 * The library reaches the operating system only through the __rtos_*
 * functions below. Exactly one port library (librtos-<port>.a) defines them;
 * they are bound at link time. The __libc_* functions are the services libc
 * provides to a port.
 *
 * Error convention: functions returning ptrdiff_t or int report failure as a
 * negated <errno.h> value, which libc stores in errno.
 */

#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* Process control */

/* Terminate the program normally with the given status; never returns */
[[noreturn]] void __rtos_exit(int status);

/* Terminate the program abnormally (abort); never returns */
[[noreturn]] void __rtos_abort(void);

/*
 * Environment (P1.05). __rtos_getenv returns the value of an environment
 * variable or a null pointer; the string belongs to the port. __rtos_system
 * runs a command as system() does: for a null pointer it returns nonzero
 * only if a command processor exists.
 */
char *__rtos_getenv(const char *name);
int __rtos_system(const char *command);

/* I/O. Handles 0, 1 and 2 are the standard input, output and error streams */

/* Read up to count bytes; returns the number read, 0 at end of file, or a negated errno */
ptrdiff_t __rtos_read(int handle, void *buffer, size_t count);

/* Write up to count bytes; returns the number written or a negated errno */
ptrdiff_t __rtos_write(int handle, const void *buffer, size_t count);

/*
 * Files (P1.09, docs/research/stdio.md). __rtos_open returns a new handle or
 * a negated errno. The flags combine the values below: READ and/or WRITE;
 * APPEND makes every write go to the end; CREATE creates a missing file,
 * EXCLUSIVE fails if it exists; TRUNCATE empties it; TEMPORARY files are
 * removed when closed. A port without a file system returns -ENOSYS.
 */
#define __RTOS_OPEN_READ 0x01
#define __RTOS_OPEN_WRITE 0x02
#define __RTOS_OPEN_APPEND 0x04
#define __RTOS_OPEN_CREATE 0x08
#define __RTOS_OPEN_TRUNCATE 0x10
#define __RTOS_OPEN_EXCLUSIVE 0x20
#define __RTOS_OPEN_TEMPORARY 0x40

int __rtos_open(const char *path, unsigned int flags);

/* Returns 0 or a negated errno */
int __rtos_close(int handle);

/* whence is SEEK_SET, SEEK_CUR or SEEK_END (0, 1, 2); returns the new offset or a negated errno */
long long __rtos_seek(int handle, long long offset, int whence);

/* Return 0 or a negated errno */
int __rtos_remove(const char *path);
int __rtos_rename(const char *old_path, const char *new_path);

/*
 * Store candidate name number index for a temporary file (tmpnam, tmpfile)
 * in buffer, at most size bytes including the terminator (size is L_tmpnam);
 * returns 0 or a negated errno. Names for different indices differ.
 */
int __rtos_tmpname(char *buffer, size_t size, unsigned int index);

/*
 * Clocks (P1.11). base is TIME_UTC (calendar time since the epoch),
 * TIME_MONOTONIC, TIME_ACTIVE (the program's processor time, which clock
 * reports) or TIME_THREAD_ACTIVE (the calling thread's). Return 0 or a
 * negated errno (-EINVAL for an unsupported base, -ENOSYS without a clock).
 */
int __rtos_clock_gettime(int base, struct timespec *ts);
int __rtos_clock_getres(int base, struct timespec *res);

/*
 * Heap (PLAN D31). libc's allocator asks for its memory once, at the first
 * allocation, by calling this with index 0, 1, 2, ... until it returns false
 * (at most 8 regions). Each region is a fixed block of memory the allocator
 * owns from then on; it needs no particular alignment. A port without heap
 * memory returns false for index 0, and every allocation then fails.
 *
 * An RTOS that owns its heap through an sbrk-style break (PLAN D103)
 * implements region 0 by taking one block from that break at this first
 * call: everything up to the heap's end, or all but a reserve the RTOS keeps
 * for its other sbrk users. Its other indexes return false.
 */
bool __rtos_heap_region(unsigned int index, void **base, size_t *size);

/*
 * Locks (PLAN D31, D79). The port defines struct __rtos_lock; libc only
 * handles pointers to it, so one libc.a serves every port. Locks are
 * recursive: the owning thread may acquire a lock it holds, and must release
 * it as often. libc names the statically allocated locks below, which the
 * port defines; __rtos_lock_create and __rtos_lock_destroy serve locks libc
 * creates at run time.
 *
 * The lock word (D79). struct __rtos_lock starts with an unsigned int that
 * is 0 when the lock is free and otherwise holds the owner's token, with
 * __RTOS_LOCK_SLOW set when releasing needs the port: the owner holds it
 * more than once, or another thread may be waiting. A thread's token is its
 * value of __rtos_lock_token, which the port defines and keeps nonzero,
 * even and unique among the threads that may hold a lock. libc may take a
 * free lock by an atomic compare-and-swap of 0 to its token, and give back
 * a lock held once without waiters by a compare-and-swap of its token to 0,
 * calling __rtos_lock_acquire and __rtos_lock_release only when those fail.
 * The port's functions must accept such a lock word at any time. A
 * single-threaded port may still implement them as no-ops: no other thread
 * can see that a nested release then frees the word early.
 */
struct __rtos_lock;

#define __RTOS_LOCK_SLOW 1u

extern thread_local unsigned int __rtos_lock_token;

/* Serialises the memory management functions */
extern struct __rtos_lock __rtos_malloc_lock;

/* Serialises the atexit and at_quick_exit tables */
extern struct __rtos_lock __rtos_atexit_lock;

/* The standard streams' locks, and the lock of the list of open streams (P1.09) */
extern struct __rtos_lock __rtos_stdin_lock;
extern struct __rtos_lock __rtos_stdout_lock;
extern struct __rtos_lock __rtos_stderr_lock;
extern struct __rtos_lock __rtos_stdio_lock;

void __rtos_lock_acquire(struct __rtos_lock *lock);
void __rtos_lock_release(struct __rtos_lock *lock);

/* Returns 0 and stores a new lock, or a negated errno */
int __rtos_lock_create(struct __rtos_lock **lock);
void __rtos_lock_destroy(struct __rtos_lock *lock);

/*
 * Threads (P1.12, docs/research/threads.md). The port defines struct
 * __rtos_thread; <threads.h> identifies a thread by a pointer to it. A new
 * thread runs entry(arg) with its own stack and TLS block (initialised with
 * __libc_tls_init); if entry returns, the thread ends with its result.
 * __rtos_thread_self works in every thread, the initial one included.
 * Functions returning int give 0 or a negated errno; __rtos_thread_sleep
 * returns -EINTR, with the time left in remaining when that is not null, if
 * a signal ends the sleep early.
 */
struct __rtos_thread;

int __rtos_thread_create(struct __rtos_thread **thread, int (*entry)(void *), void *arg);
[[noreturn]] void __rtos_thread_exit(int result);
int __rtos_thread_join(struct __rtos_thread *thread, int *result);
int __rtos_thread_detach(struct __rtos_thread *thread);
struct __rtos_thread *__rtos_thread_self(void);
void __rtos_thread_yield(void);
int __rtos_thread_sleep(const struct timespec *duration, struct timespec *remaining);

/*
 * Wait and wake on an address (P1.12). __rtos_futex_wait blocks the caller
 * while *address equals expected, until a wake for that address, the
 * TIME_UTC deadline (null: none) or an unspecified reason; it returns 0
 * after a wake or early return, -EAGAIN if *address differed on entry and
 * -ETIMEDOUT once the deadline passed. __rtos_futex_wake unblocks up to
 * count waiters (UINT_MAX: all) and returns how many, or a negated errno.
 * libc builds mutexes, condition variables and call_once on these.
 */
int __rtos_futex_wait(unsigned int *address, unsigned int expected, const struct timespec *deadline);
int __rtos_futex_wake(unsigned int *address, unsigned int count);

/*
 * Support for the __atomic_* library functions libc provides for operations
 * the compiler cannot inline (P1.12, docs/research/threads.md).
 *
 * __rtos_atomic_cas atomically replaces *address by desired if it holds
 * expected, and reports whether it did. It must be atomic against the plain
 * loads and stores the compiler inlines for 4-byte atomics (on armv6-m, an
 * interrupt mask on one core, or the kernel's helper under Linux).
 *
 * The critical section serves 8-byte and odd-sized atomics, which only the
 * library accesses: while it is held no other such operation may run, in
 * any thread or interrupt handler; it does not nest. enter returns a state
 * that exit restores (e.g. the interrupt mask).
 */
bool __rtos_atomic_cas(volatile unsigned int *address, unsigned int expected, unsigned int desired);
unsigned long __rtos_atomic_enter(void);
void __rtos_atomic_exit(unsigned long state);

/*
 * Thread-local storage (PLAN D10). A port describes the TLS initialization
 * image (the PT_TLS segment, or the equivalent linker symbols) and, for every
 * thread including the initial one, provides a block of
 * __libc_tls_block_size() bytes with no alignment requirement. libc lays out
 * the TLS data and the thread control block inside it for the arch's TLS
 * variant, and returns the value the port must install as the thread pointer.
 */
struct __libc_tls_image {
	const void *init; /* initialization image (.tdata); its address fixes the alignment phase */
	size_t init_size; /* bytes to copy from init */
	size_t size; /* total bytes (.tdata + .tbss); the rest is zero filled */
	size_t align; /* alignment of the TLS segment, a power of two (0 means 1) */
};

size_t __libc_tls_block_size(const struct __libc_tls_image *image);
void *__libc_tls_init(void *block, const struct __libc_tls_image *image);

/*
 * Program start. The port's startup code establishes the stack and the
 * initial thread's TLS, then calls this. It runs the static constructors
 * (.init_array), calls main and passes its result to exit.
 *
 * .preinit_array belongs to the port (PLAN D104): it is an ELF convention,
 * not a C or compiler requirement, and an RTOS may have early hooks of its
 * own there. A port that supports the convention runs the table, whose
 * bounds the default linker scripts define, before calling
 * __libc_start_main; libc never runs it.
 */
[[noreturn]] void __libc_start_main(int (*main)(int, char **), int argc, char **argv);

extern void (*const __preinit_array_start[])(void);
extern void (*const __preinit_array_end[])(void);

__LIBC_END_DECLS

#endif /* _RTOS_RTOS_PORT_H */
