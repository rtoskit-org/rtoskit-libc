/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fcmple.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a <= b, 1 or 0 (IEEE 754-2019 5.11, RTABI 4.1.2; PLAN D109): false when
 * either operand is a NaN (unordered is C set, Z clear: higher).
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fcmple,"ax",%progbits
	.global __aeabi_fcmple
	.type __aeabi_fcmple, %function
__aeabi_fcmple:
	push {r4, lr}
	bl __libc_softfloat_compare
	ite ls
	movls r0, #1
	movhi r0, #0
	pop {r4, pc}
	.size __aeabi_fcmple, . - __aeabi_fcmple

	.section .note.GNU-stack, "", %progbits
