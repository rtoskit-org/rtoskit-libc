/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_ul2f.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * (float)a for an unsigned long long (C23 6.3.1.4p2, RTABI 4.1.2; PLAN
 * D109), round to nearest, ties to even (D44): the leading one moved to
 * bit 63 by CLZ, the 24 bits below it rounded on the guard bit with
 * everything lower sticky; the exponent field is 190 - k. UINT64_MAX
 * rounds up to 2^64.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_ul2f,"ax",%progbits
	.global __aeabi_ul2f
	.type __aeabi_ul2f, %function
__aeabi_ul2f:
	movs ip, #0
	/* A zero high word: the low word moves up, 32 places less of exponent; zero stays +0 */
	movw r3, #189
	cbnz r1, 1f
	movs r1, r0
	beq 2f
	movs r0, #0
	movw r3, #157
1:
	/* The leading one to bit 63, the low word's remaining bits a sticky bit 0 of the high word */
	clz r2, r1
	sub r3, r3, r2
	add ip, ip, r3, lsl #23
	lsl r1, r1, r2
	rsb r3, r2, #32
	lsr r3, r0, r3
	orr r1, r1, r3
	lsls r0, r0, r2
	it ne
	orrne r1, r1, #1

	/* sign | (exponent - 1) 2^23 + bits 31-8, the guard bit as carry in; a tie clears the last bit */
	and r3, r1, #0xff
	lsrs r1, r1, #8
	adc r0, ip, r1
	cmp r3, #0x80
	it eq
	biceq r0, r0, #1
2:
	bx lr
	.size __aeabi_ul2f, . - __aeabi_ul2f

	.section .note.GNU-stack, "", %progbits
