/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strlen.S
 *
 * Created on: Oct 26, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * C23 7.26.6.4 for Thumb-2 (Armv7-M, Armv7E-M, Armv8-M Mainline), written
 * for speed (PLAN D112, docs/research/strlen-thumb2.md). Bytes until the
 * pointer is 8-byte aligned, then two words per LDRD: aligned to 8, each read
 * stays inside one 32-byte MPU granule, the smallest region (LDRD itself needs
 * word alignment, Armv7-M ARM A3.2.1). Each word gets the classic test
 * (w - 0x01010101) & ~w & 0x80808080, whose lowest set bit marks the first
 * zero byte exactly (both constants are immediates, so nothing is pushed),
 * and its own exit: RBIT and CLZ give 8 times that byte's index. 4.5
 * instructions a word against the Thumb-1 loop's 5. r12 keeps the start.
 */
	.syntax unified
	.thumb
	.text
	.global strlen
	.type strlen, %function
strlen:
	mov ip, r0
1:
	tst r0, #7
	beq 2f
	ldrb r1, [r0], #1
	cmp r1, #0
	bne 1b
	sub r0, r0, ip
	subs r0, #1
	bx lr
2:
	ldrd r1, r2, [r0], #8
	sub r3, r1, #0x01010101
	bic r3, r3, r1
	tst r3, #0x80808080
	bne 4f
	sub r3, r2, #0x01010101
	bic r3, r3, r2
	ands r3, r3, #0x80808080
	beq 2b
3:
	/* The zero byte is in the word at r0 - 4: its index is the flags' trailing zeros / 8 */
	rbit r3, r3
	clz r3, r3
	sub r0, r0, ip
	add r0, r0, r3, lsr #3
	subs r0, #4
	bx lr
4:
	/* In the word at r0 - 8: as for the second word, 4 bytes earlier (TST left the flags in r3 unmasked, and RBIT/CLZ only need the lowest) */
	and r3, r3, #0x80808080
	subs r0, #4
	b 3b
	.size strlen, . - strlen

	.section .note.GNU-stack, "", %progbits
