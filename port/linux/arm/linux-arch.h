/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * linux-arch.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _LINUX_ARCH_H
#define _LINUX_ARCH_H

#include <stdint.h>

/*
 * Arm EABI Linux system call interface (the same for every M-profile
 * architecture): the number goes in r7, arguments in r0-r5, the result comes
 * back in r0. Errors are returned as -4095..-1. The calls go through the
 * out-of-line __rtos_linux_syscall.
 */

#define __LINUX_SYS_CLOSE 6
#define __LINUX_SYS_EXIT_GROUP 248
#define __LINUX_SYS_GETPID 20
#define __LINUX_SYS_GETTID 224
#define __LINUX_SYS_LLSEEK 140
#define __LINUX_SYS_OPENAT 322
#define __LINUX_SYS_READ 3
#define __LINUX_SYS_RENAMEAT 329
#define __LINUX_SYS_TGKILL 268
#define __LINUX_SYS_UNLINKAT 328
#define __LINUX_SYS_WRITE 4

/* Files beyond 2 GiB need the flag on a 32-bit kernel interface */
/* The _time64 calls, which take a 64-bit struct timespec */
#define __LINUX_SYS_CLOCK_GETTIME 403
#define __LINUX_SYS_CLOCK_GETRES 406

/* Threads (P1.12): the time64 futex and clock_nanosleep take the 64-bit timespec */
#define __LINUX_SYS_EXIT 1
#define __LINUX_SYS_MUNMAP 91
#define __LINUX_SYS_CLONE 120
#define __LINUX_SYS_SCHED_YIELD 158

/* getentropy (issue #127) */
#define __LINUX_SYS_GETRANDOM 384
#define __LINUX_SYS_CLOCK_NANOSLEEP 407
#define __LINUX_SYS_FUTEX 422

#define __LINUX_O_LARGEFILE 0x20000

/* mmap2: the offset is in 4096-byte units, which makes no difference for the offset 0 used here */
#define __LINUX_SYS_MMAP 192

/* Arm private calls */
#define __LINUX_ARM_SET_TLS 0xf0005

long __rtos_linux_syscall(long a, long b, long c, long d, long e, long f, long number);

/* Provided by the port for thread_local access (PLAN D10) */
void *__aeabi_read_tp(void);

/*
 * The kernel's compare-and-swap helper (Documentation/arm/kernel_user_helpers.rst):
 * returns 0 when *address held expected and now holds desired. Barriers included.
 */
static inline int __linux_kuser_cmpxchg(unsigned int expected, unsigned int desired, volatile unsigned int *address)
{
	int (*cmpxchg)(unsigned int, unsigned int, volatile unsigned int *) = (int (*)(unsigned int, unsigned int, volatile unsigned int *))0xffff0fc0;

	return cmpxchg(expected, desired, address);
}

static inline long __linux_syscall0(long number)
{
	return __rtos_linux_syscall(0, 0, 0, 0, 0, 0, number);
}

static inline long __linux_syscall1(long number, long a)
{
	return __rtos_linux_syscall(a, 0, 0, 0, 0, 0, number);
}

static inline long __linux_syscall2(long number, long a, long b)
{
	return __rtos_linux_syscall(a, b, 0, 0, 0, 0, number);
}

static inline long __linux_syscall3(long number, long a, long b, long c)
{
	return __rtos_linux_syscall(a, b, c, 0, 0, 0, number);
}

static inline long __linux_syscall6(long number, long a, long b, long c, long d, long e, long f)
{
	return __rtos_linux_syscall(a, b, c, d, e, f, number);
}

/* _llseek takes the offset in two halves and stores the 64-bit result */
static inline long long __linux_seek(int handle, long long offset, int whence)
{
	long long result;
	long status = __linux_syscall6(__LINUX_SYS_LLSEEK, handle, (long)(offset >> 32), (long)(offset & 0xffffffff), (long)&result, whence, 0);

	return status < 0 ? status : result;
}

/* ELF32 file and program headers, only the fields startup reads are meaningful */
struct __linux_ehdr {
	unsigned char e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct __linux_phdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

#endif /* _LINUX_ARCH_H */
