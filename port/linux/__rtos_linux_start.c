/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __rtos_linux_start.c
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <rtos/rtos-port.h>

#include "linux.h"

char **__rtos_linux_environ;
struct __libc_tls_image __rtos_linux_tls_image;

/* Find the PT_TLS segment of the (static, non-PIE) executable */
static void find_tls_image(struct __libc_tls_image *image)
{
	const unsigned char *phdrs = (const unsigned char *)&__ehdr_start + __ehdr_start.e_phoff;

	for (unsigned int i = 0; i < __ehdr_start.e_phnum; ++i) {
		const struct __linux_phdr *phdr = (const struct __linux_phdr *)(phdrs + i * __ehdr_start.e_phentsize);

		if (phdr->p_type == __LINUX_PT_TLS) {
			image->init = (const void *)(uintptr_t)phdr->p_vaddr;
			image->init_size = phdr->p_filesz;
			image->size = phdr->p_memsz;
			image->align = phdr->p_align;
			return;
		}
	}
}

/* The initial thread's TLS must exist before any libc code touches errno */
static void setup_initial_tls(void)
{
	struct __libc_tls_image *image = &__rtos_linux_tls_image;

	image->init = &__ehdr_start;
	find_tls_image(image);

	long block = __linux_syscall6(__LINUX_SYS_MMAP, 0, (long)__libc_tls_block_size(image),
								  __LINUX_PROT_READ | __LINUX_PROT_WRITE, __LINUX_MAP_PRIVATE | __LINUX_MAP_ANONYMOUS, -1, 0);
	if (__linux_is_error(block))
		__linux_syscall1(__LINUX_SYS_EXIT_GROUP, 127);

	__rtos_linux_set_tp(__libc_tls_init((void *)block, image));
}

void __rtos_linux_start(long *stack)
{
	int argc = (int)stack[0];
	char **argv = (char **)(stack + 1);

	__rtos_linux_environ = argv + argc + 1;

	setup_initial_tls();

	/* The port runs .preinit_array (PLAN D104), with the TLS already in place */
	for (void (*const *entry)(void) = __preinit_array_start; entry < __preinit_array_end; ++entry)
		(*entry)();

	__libc_start_main(main, argc, argv);
}
