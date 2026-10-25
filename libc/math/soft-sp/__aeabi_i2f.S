/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_i2f.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * (float)a for an int (C23 6.3.1.4p2, RTABI 4.1.2; PLAN D109): the sign,
 * then __aeabi_ui2f's rounding of the magnitude (INT_MIN's is 2^31 as an
 * unsigned word).
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_i2f,"ax",%progbits
	.global __aeabi_i2f
	.type __aeabi_i2f, %function
__aeabi_i2f:
	ands r3, r0, #0x80000000
	it mi
	rsbmi r0, r0, #0
	clz r1, r0
	lsls r0, r0, r1
	beq 1f
	rsb r1, r1, #157
	add r1, r3, r1, lsl #23
	and r2, r0, #0xff
	lsrs r0, r0, #8
	adc r0, r1, r0
	cmp r2, #0x80
	it eq
	biceq r0, r0, #1
1:
	bx lr
	.size __aeabi_i2f, . - __aeabi_i2f

	.section .note.GNU-stack, "", %progbits
