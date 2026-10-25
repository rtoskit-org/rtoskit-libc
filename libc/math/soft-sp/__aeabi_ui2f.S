/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_ui2f.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * (float)a for an unsigned int (C23 6.3.1.4p2, RTABI 4.1.2; PLAN D109),
 * round to nearest, ties to even (D44): the leading one moved to bit 31
 * by CLZ, then bits 31-8 are the significand, whose hidden bit restores
 * the exponent field 158 - k from 157 - k, with bit 7 (the guard bit) as
 * the carry in; a tie (bits 6-0 zero) clears the last bit. Zero stays +0.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_ui2f,"ax",%progbits
	.global __aeabi_ui2f
	.type __aeabi_ui2f, %function
__aeabi_ui2f:
	clz r1, r0
	lsls r0, r0, r1
	beq 1f
	rsb r1, r1, #157
	lsls r1, r1, #23
	and r2, r0, #0xff
	lsrs r0, r0, #8
	adc r0, r1, r0
	cmp r2, #0x80
	it eq
	biceq r0, r0, #1
1:
	bx lr
	.size __aeabi_ui2f, . - __aeabi_ui2f

	.section .note.GNU-stack, "", %progbits
