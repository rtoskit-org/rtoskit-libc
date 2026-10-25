/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * longjmp.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * C23 7.13.2.1 for Armv7-M: restore what setjmp saved and return from that
 * setjmp again, with val, or 1 when val is 0 (p4).
 */
	.syntax unified
	.thumb
	.text
	.global longjmp
	.type longjmp, %function
longjmp:
	ldmia r0, {r4-r11, ip, lr}
	mov sp, ip
	movs r0, r1
	it eq
	moveq r0, #1
	bx lr
	.size longjmp, . - longjmp

	.section .note.GNU-stack, "", %progbits
