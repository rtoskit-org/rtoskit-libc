/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * memchr.S
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * C23 7.26.5.2 for Thumb-2 (Armv7-M, Armv7E-M, Armv8-M Mainline), written
 * for speed (PLAN D113, docs/research/memchr-thumb2.md). The character
 * converts to unsigned char (7.26.5.2p2) and is replicated into r1, whose top
 * byte the byte loops compare against. Bytes until the pointer is word
 * aligned; then aligned words, two per iteration, while 4 or more bytes
 * remain, each XORed with the pattern and given the classic test (x -
 * 0x01010101) & ~x & 0x80808080, whose lowest set bit marks the first match
 * exactly; then bytes again. Only aligned words wholly inside the n bytes are
 * read, each inside one 32-byte MPU granule, so a read past a match (where n
 * may run past the object: the search stops at the first match, 7.26.5.2p2)
 * never leaves the granule holding it. RBIT and CLZ give 8 times the
 * matching byte's index. Nothing is pushed.
 */
	.syntax unified
	.thumb
	.text
	.global memchr
	.type memchr, %function
memchr:
	uxtb r1, r1
	orr r1, r1, r1, lsl #8
	orr r1, r1, r1, lsl #16
1:
	cmp r2, #0
	beq 8f
	tst r0, #3
	beq 2f
	ldrb r3, [r0]
	cmp r3, r1, lsr #24
	beq 9f
	adds r0, #1
	subs r2, #1
	b 1b
2:
	/* Word aligned: two words per iteration while 8 bytes remain */
	subs r2, #8
	blo 5f
4:
	ldr r3, [r0], #4
	eor r3, r3, r1
	sub ip, r3, #0x01010101
	bic ip, ip, r3
	ands ip, ip, #0x80808080
	bne 10f
	ldr r3, [r0], #4
	eor r3, r3, r1
	sub ip, r3, #0x01010101
	bic ip, ip, r3
	ands ip, ip, #0x80808080
	bne 10f
	subs r2, #8
	bhs 4b
5:
	/* 0 to 7 bytes left: one more word if 4 of them remain */
	adds r2, #8
	cmp r2, #4
	blo 7f
	ldr r3, [r0], #4
	eor r3, r3, r1
	sub ip, r3, #0x01010101
	bic ip, ip, r3
	ands ip, ip, #0x80808080
	bne 10f
	subs r2, #4
7:
	cmp r2, #0
	beq 8f
	ldrb r3, [r0]
	cmp r3, r1, lsr #24
	beq 9f
	adds r0, #1
	subs r2, #1
	b 7b
8:
	movs r0, #0
9:
	bx lr
10:
	/* The match is in the word at r0 - 4 */
	rbit ip, ip
	clz ip, ip
	subs r0, #4
	add r0, r0, ip, lsr #3
	bx lr
	.size memchr, . - memchr

	.section .note.GNU-stack, "", %progbits
