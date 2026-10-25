/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fcmpeq.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a == b, 1 or 0 (IEEE 754-2019 5.11, RTABI 4.1.2; PLAN D109): false when
 * either operand is a NaN; -0 equals +0.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fcmpeq,"ax",%progbits
	.global __aeabi_fcmpeq
	.type __aeabi_fcmpeq, %function
__aeabi_fcmpeq:
	push {r4, lr}
	bl __libc_softfloat_compare
	ite eq
	moveq r0, #1
	movne r0, #0
	pop {r4, pc}
	.size __aeabi_fcmpeq, . - __aeabi_fcmpeq

	.section .note.GNU-stack, "", %progbits
