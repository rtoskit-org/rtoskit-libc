/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * setjmp.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * C23 7.13.1.1 for Armv7-M, which has no FPU (AAPCS 5.1.1): save the
 * callee-saved r4-r11, the stack pointer and the return address. The layout
 * is the start of the FPU cores' (r4-r11, sp, lr), and jmp_buf is sized for
 * the largest variant (PLAN D15, D108).
 */
	.syntax unified
	.thumb
	.text
	.global setjmp
	.type setjmp, %function
setjmp:
	mov ip, sp
	stmia r0, {r4-r11, ip, lr}
	movs r0, #0
	bx lr
	.size setjmp, . - setjmp

	.section .note.GNU-stack, "", %progbits
