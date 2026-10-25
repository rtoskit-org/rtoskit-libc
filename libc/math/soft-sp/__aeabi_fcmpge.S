/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fcmpge.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a >= b, 1 or 0 (IEEE 754-2019 5.11, RTABI 4.1.2; PLAN D109): b <= a, so
 * that unordered (C set, Z clear) reads as false.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fcmpge,"ax",%progbits
	.global __aeabi_fcmpge
	.type __aeabi_fcmpge, %function
__aeabi_fcmpge:
	push {r4, lr}
	mov r2, r0
	mov r0, r1
	mov r1, r2
	bl __libc_softfloat_compare
	ite ls
	movls r0, #1
	movhi r0, #0
	pop {r4, pc}
	.size __aeabi_fcmpge, . - __aeabi_fcmpge

	.section .note.GNU-stack, "", %progbits
