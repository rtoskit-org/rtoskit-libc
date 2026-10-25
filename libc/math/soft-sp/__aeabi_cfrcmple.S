/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_cfrcmple.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The ordering of b and a in the flags (RTABI 4.1.2, the reversed
 * __aeabi_cfcmple; PLAN D109): the operands swapped around the compare
 * core and back, with MOVs that leave the flags alone, so r0-r3 are kept.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_cfrcmple,"ax",%progbits
	.global __aeabi_cfrcmple
	.type __aeabi_cfrcmple, %function
__aeabi_cfrcmple:
	push {r4, lr}
	mov ip, r0
	mov r0, r1
	mov r1, ip
	bl __libc_softfloat_compare
	mov ip, r0
	mov r0, r1
	mov r1, ip
	pop {r4, pc}
	.size __aeabi_cfrcmple, . - __aeabi_cfrcmple

	.section .note.GNU-stack, "", %progbits
