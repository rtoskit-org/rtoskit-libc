/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strchr.S
 *
 * Created on: Oct 20, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * C23 7.26.5.2 for Thumb-2 with the DSP extension (Armv7E-M, Armv8-M
 * Mainline +dsp), written for speed (PLAN D74, docs/research/string-speed.md).
 * The character converts to char (7.26.5.2p2: unsigned char here). Bytes are
 * tested up to a word boundary, then a word at a time: UADD8 with 0xffffffff
 * sets GE[i] for the non-zero bytes and SEL makes 0xff of every zero byte;
 * the word XOR the replicated character has zero bytes where it matches,
 * and a second UADD8/SEL keeps the first mask where the byte differs and
 * puts 0xff where it matches. Only aligned words are read, none past the one
 * holding the terminator. The first flagged byte (REV, CLZ) is either the
 * character, whose address is the result, or the terminator: a null pointer,
 * unless the character is the terminator itself.
 */
	.syntax unified
	.thumb
	.text
	.global strchr
	.type strchr, %function
strchr:
	uxtb r1, r1
1:
	tst r0, #3
	beq 3f
	ldrb r2, [r0]
	cmp r2, r1
	beq 2f
	adds r0, r0, #1
	cmp r2, #0
	bne 1b
	movs r0, #0
2:
	bx lr

3:
	push {r4, r5}
	mov r4, #0
	mvn r5, #0
	orr ip, r1, r1, lsl #8
	orr ip, ip, ip, lsl #16
4:
	ldr r2, [r0], #4
	uadd8 r3, r2, r5
	sel r3, r4, r5
	eor r2, r2, ip
	uadd8 r2, r2, r5
	sel r3, r3, r5
	cmp r3, #0
	beq 4b
	pop {r4, r5}
	rev r3, r3
	clz r3, r3
	subs r0, r0, #4
	add r0, r0, r3, lsr #3
	ldrb r2, [r0]
	cmp r2, r1
	it ne
	movne r0, #0
	bx lr
	.size strchr, . - strchr

	.section .note.GNU-stack, "", %progbits
