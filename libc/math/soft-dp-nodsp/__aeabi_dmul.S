/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_dmul.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a * b (IEEE 754-2019 5.4.1, RTABI 4.1.2) for Armv7-M, which has no DSP
 * extension and so no UMAAL: soft-dp's routine with the two UMAALs written
 * with base instructions (PLAN D99, D108, docs/research/armv7-m.md). Round
 * to nearest, no exception flags (D44).
 *
 * a is r1:r0 and b r3:r2 (base calling convention). a's significand moved
 * to the top of 64 bits (A = r7:r6) times b's 53 bits (r8:r2) is the
 * 117-bit product, from one UMULL and three UMLALs; the last cross product
 * takes its second addend through ADDS/ADC first, since a 32 x 32 product
 * plus two words still fits in 64 bits. Its upper half H = r9:lr lies in
 * [2^51, 2^53): the significand, after one left shift when it is below
 * 2^52. The lower half L = ip:r5 supplies the rounding: its top bit is
 * half an ulp, the rest sticky, so r5 is folded into ip's last bit first.
 * The rounding is to nearest, and a tie clears the last bit, for the even
 * neighbour; a carry moves into the exponent, and to infinity at the top.
 *
 * A subnormal operand is normalised first, its exponent then below that
 * of field 1. A result above the range is infinity; one below it is
 * shifted right onto the subnormal grid, the bits shifted out kept as a
 * rounding bit and a sticky bit, and rounded once. NaN operands give the
 * first NaN, quietened (IEEE 754-2019 6.2.3); infinity times zero is the
 * default NaN (7.2); a zero times a finite operand is a zero signed by the
 * signs' difference (6.3).
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_dmul,"ax",%progbits
	.global __aeabi_dmul
	.type __aeabi_dmul, %function
__aeabi_dmul:
	push {r4-r9, lr}

	/* Exponent fields; 0 (zero, subnormal) and 2047 (infinity, NaN) are special */
	ubfx r4, r1, #20, #11
	ubfx r5, r3, #20, #11
	movw lr, #2046
	sub ip, r4, #1
	cmp ip, lr
	bhs .Lspecial
	sub ip, r5, #1
	cmp ip, lr
	bhs .Lspecial
	add r4, r4, r5

.Lproduct:
	/* A = a's significand 2^11 (hidden bit at 63), b's high word with its hidden bit */
	lsls r7, r1, #11
	orr r7, r7, r0, lsr #21
	orr r7, r7, #0x80000000
	lsls r6, r0, #11
	ubfx r8, r3, #0, #20
	orr r8, r8, #0x00100000

	/* r9:lr:ip:r5 = A (r8:r2) */
	umull r5, ip, r6, r2
	movs lr, #0
	umlal ip, lr, r7, r2
	movs r9, #0
	umlal ip, r9, r6, r8
	adds lr, lr, r9
	mov r9, #0
	adc r9, r9, #0
	umlal lr, r9, r7, r8

	/* Only whether L's low word is zero matters */
	cmp r5, #0
	it ne
	orrne ip, ip, #1

	/* H below 2^52: shift H:L left one place, and the exponent down one */
	tst r9, #0x00100000
	bne 1f
	adds ip, ip, ip
	adcs lr, lr, lr
	adc r9, r9, r9
	subs r4, r4, #1
1:
	/* The biased exponent less one, ea + eb - 1023, must lie in [0, 2045] */
	subw r4, r4, #1023
	movw r5, #2045
	cmp r4, r5
	bhi .Lrange

.Lpack:
	/* sign | (exponent - 1) 2^20 + H: the hidden bit in H adds the one back */
	eor r5, r1, r3
	and r5, r5, #0x80000000
	add r5, r5, r4, lsl #20
	add r5, r5, r9

	/* Round to nearest: add half an ulp's carry; a tie (the rest zero) clears the last bit, for the even neighbour */
	lsls r6, ip, #1
	bcc 2f
	adds lr, lr, #1
	adc r5, r5, #0
	cmp r6, #0
	it eq
	biceq lr, lr, #1
2:
	mov r0, lr
	mov r1, r5
	pop {r4-r9, pc}

.Lrange:
	/* Above the range: infinity */
	cmp r4, #0
	blt 3f
	eor r1, r1, r3
	and r1, r1, #0x80000000
	orr r1, r1, #0x7f000000
	orr r1, r1, #0x00f00000
	movs r0, #0
	pop {r4-r9, pc}
3:
	/*
	 * Below it: shift H:L right s = -r4 places (at most 63) onto the
	 * subnormal grid, ip keeping the top bit shifted out and a sticky bit
	 * for the rest, then pack with field 0
	 */
	rsbs r5, r4, #0
	cmp r5, #63
	it hi
	movhi r5, #63
	movs r4, #0
	cmp ip, #0
	it ne
	movne ip, #1
	cmp r5, #32
	bhs 4f
	rsb r6, r5, #32
	lsl r7, lr, r6
	orr ip, ip, r7
	lsr lr, lr, r5
	lsl r7, r9, r6
	orr lr, lr, r7
	lsr r9, r9, r5
	b .Lpack
4:
	subs r5, r5, #32
	rsb r6, r5, #32
	lsls r7, lr, r6
	it ne
	orrne ip, ip, #1
	lsl r7, r9, r6
	orr ip, ip, r7
	lsr r7, lr, r5
	orr ip, ip, r7
	lsr lr, r9, r5
	movs r9, #0
	b .Lpack

.Lspecial:
	/* A zero times a finite operand: a zero signed by the signs' difference (IEEE 754-2019 6.3) */
	orrs ip, r2, r3, lsl #1
	beq 5f
	orrs ip, r0, r1, lsl #1
	bne 6f
	mov ip, r5
	b 7f
5:
	mov ip, r4
7:
	add ip, ip, #1
	tst ip, #0x800
	bne 6f
	eor r1, r1, r3
	and r1, r1, #0x80000000
	movs r0, #0
	pop {r4-r9, pc}
6:
	/* A NaN operand, quietened: a's if a is one */
	add ip, r4, #1
	tst ip, #0x800
	beq 8f
	orrs ip, r0, r1, lsl #12
	itt ne
	orrne r1, r1, #0x00080000
	popne {r4-r9, pc}
8:
	add ip, r5, #1
	tst ip, #0x800
	beq 9f
	orrs ip, r2, r3, lsl #12
	beq 9f
	mov r0, r2
	orr r1, r3, #0x00080000
	pop {r4-r9, pc}
9:
	/* r6 = the result's sign, r8 = 1 if either operand is a zero (the other then infinite) */
	eor r6, r1, r3
	and r6, r6, #0x80000000
	movs r8, #0
	orrs ip, r0, r1, lsl #1
	it eq
	moveq r8, #1
	orrs ip, r2, r3, lsl #1
	it eq
	moveq r8, #1

	/* An infinity: times zero invalid, the default NaN; otherwise infinity */
	add ip, r4, #1
	add r7, r5, #1
	orr ip, ip, r7
	tst ip, #0x800
	beq 17f
	movs r0, #0
	cmp r8, #0
	bne 18f
	orr r1, r6, #0x7f000000
	orr r1, r1, #0x00f00000
	pop {r4-r9, pc}
18:
	movs r1, #0x7f000000
	orr r1, r1, #0x00f80000
	pop {r4-r9, pc}
17:
	/* A zero times a finite operand */
	cmp r8, #0
	beq 19f
	mov r1, r6
	movs r0, #0
	pop {r4-r9, pc}
19:
	/* Subnormal operands: shift the leading bit to the hidden bit's place, s places, and the exponent to 1 - s */
	cbnz r4, 20f
	ubfx ip, r1, #0, #20
	bl .Lnormalise
	bfi r1, ip, #0, #20
	mov r4, r7
20:
	cbnz r5, 21f
	mov r6, r0
	mov r0, r2
	ubfx ip, r3, #0, #20
	bl .Lnormalise
	bfi r3, ip, #0, #20
	mov r2, r0
	mov r0, r6
	mov r5, r7
21:
	/* .Lproduct takes the significands from the low 20 bits of the high words and the low words */
	add r4, r4, r5
	b .Lproduct

	/* ip:r0, nonzero below 2^52, shifted left s places to [2^52, 2^53); r7 = 1 - s; uses r8 */
.Lnormalise:
	cmp ip, #0
	beq 22f
	clz r7, ip
	subs r7, r7, #11
	rsb r8, r7, #32
	lsls ip, ip, r7
	lsr r8, r0, r8
	orr ip, ip, r8
	lsls r0, r0, r7
	rsb r7, r7, #1
	bx lr
22:
	clz r7, r0
	adds r7, r7, #21
	cmp r7, #32
	bhs 23f
	rsb r8, r7, #32
	lsr ip, r0, r8
	lsls r0, r0, r7
	rsb r7, r7, #1
	bx lr
23:
	sub r8, r7, #32
	lsl ip, r0, r8
	movs r0, #0
	rsb r7, r7, #1
	bx lr
	.size __aeabi_dmul, . - __aeabi_dmul
