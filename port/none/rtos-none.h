/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * rtos-none.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _RTOS_NONE_H
#define _RTOS_NONE_H

#include <rtos/rtos-port.h>
#include <stddef.h>

/*
 * Bare-metal, single-threaded port. The standard output and error streams go
 * to __rtos_none_output, a weak default that discards the bytes; a board
 * replaces it (e.g. with a UART driver) by defining its own.
 */
void __rtos_none_output(const void *buffer, size_t count);

/*
 * Bare metal has no entropy source: __rtos_entropy is a weak default
 * returning -ENOSYS, so getentropy (and C++'s std::random_device) fails. A
 * board with a hardware random number generator defines its own (issue
 * #127, PLAN D146).
 */

/*
 * The heap is the memory between these symbols, which the board's linker
 * script defines. They are weak: without them there is no heap, and a board
 * may instead replace __rtos_heap_region itself to describe several regions.
 */
[[gnu::weak]] extern char __rtos_none_heap_start[];
[[gnu::weak]] extern char __rtos_none_heap_end[];

/*
 * Stack capture (std::stacktrace, _Unwind_Backtrace; P7.04, PLAN D134) on Arm:
 * libgcc's unwinder finds the EHABI index through __exidx_start and
 * __exidx_end, which the board's linker script defines around the index,
 * as the toolchain's default script does:
 *
 *	.ARM.exidx : { __exidx_start = .; *(.ARM.exidx* .gnu.linkonce.armexidx.*) __exidx_end = .; } > FLASH
 *
 * Library frames unwind only in a library built with LIBC_UNWIND_TABLES=1.
 */

/* Single-threaded: locks need no state */
struct __rtos_lock {
	unsigned int word; /* the seam's lock word (rtos-port.h, PLAN D79); the locks themselves are no-ops */
};

/* The one thread (P1.12) */
struct __rtos_thread {
	char unused;
};

/*
 * Program start (P1.12, docs/research/threads.md). The board's reset code
 * initialises memory, describes the TLS image from its linker script
 * symbols and calls __rtos_none_start, which places the thread's TLS block
 * in a static buffer of __RTOS_NONE_TLS_BLOCK_SIZE bytes (aborting if the
 * image needs more), installs the thread pointer, runs .preinit_array
 * (PLAN D104) and runs main through __libc_start_main.
 */
#define __RTOS_NONE_TLS_BLOCK_SIZE 1024

[[noreturn]] void __rtos_none_start(const struct __libc_tls_image *tls);

/*
 * Installs the thread pointer: on Arm a variable __aeabi_read_tp returns,
 * on x86_64 the %fs base (wrfsbase, which the board must enable).
 */
void __rtos_none_set_tp(void *tp);

/* The program */
extern int main(int argc, char **argv);

#endif /* _RTOS_NONE_H */
