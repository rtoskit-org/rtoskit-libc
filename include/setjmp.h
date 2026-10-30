/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * setjmp.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _SETJMP_H
#define _SETJMP_H

#include <__cdefs.h>

__LIBC_BEGIN_DECLS

/* C23 7.13 */
#define __STDC_VERSION_SETJMP_H__ 202311L

/*
 * Room for the callee-saved state of every supported architecture: 10
 * pointer-sized registers (x86_64 uses 8; Arm saves r4-r11, sp and lr) plus
 * 64 bytes (Arm's d8-d15). One expression, so the header is the same for
 * every target (PLAN D15, D33).
 */
typedef struct {
	alignas(8) unsigned char __state[10 * sizeof(void *) + 64];
} jmp_buf[1];

[[gnu::returns_twice]] int setjmp(jmp_buf env);
[[noreturn]] void longjmp(jmp_buf env, int val);

/* C23 7.13.1.1p2: setjmp may be a macro */
#define setjmp setjmp

__LIBC_END_DECLS

#endif /* _SETJMP_H */
