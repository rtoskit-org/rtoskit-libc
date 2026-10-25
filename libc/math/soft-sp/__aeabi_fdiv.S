/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fdiv.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a / b (IEEE 754-2019 5.4.1, RTABI 4.1.2) for Armv7-M, round to nearest,
 * ties to even, no exception flags (PLAN D44, D109,
 * docs/research/softfloat-thumb2.md).
 *
 * The significands with the hidden bit at bit 23, a doubled when below b,
 * so a / b lies in [1, 2). Four UDIV/MLS steps give 32 quotient bits: the
 * first divides a 2^7 (below 2^32) for 8 bits with the leading one at
 * bit 7, and each later one divides the remainder (below b, so below
 * 2^24) times 2^8 for 8 more. The quotient's leading one is then at
 * bit 31 and a non-zero remainder is the sticky bit, the layout of
 * __aeabi_fmul's product, whose packing this repeats. A NaN gives the
 * first NaN, quiet; 0/0 and infinity/infinity are the default NaN (7.2);
 * x/0 is an infinity (7.3), x/infinity a zero, each with the signs'
 * difference.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fdiv,"ax",%progbits
	.global __aeabi_fdiv
	.type __aeabi_fdiv, %function
__aeabi_fdiv:
	push {r4, r5}
	eor ip, r0, r1
	ubfx r2, r0, #23, #8
	ubfx r3, r1, #23, #8

	/* An exponent field of 0 or 255 on either side: zero, subnormal, infinity or NaN */
	sub r4, r2, #1
	cmp r4, #254
	bhs .Lspecial
	sub r4, r3, #1
	cmp r4, #254
	bhs .Lspecial
	ubfx r4, r0, #0, #23
	orr r4, r4, #0x00800000
	ubfx r5, r1, #0, #23
	orr r5, r5, #0x00800000

.Lquotient:
	/* e = ea - eb + 127 for a quotient in [1, 2): a below b doubles, one less */
	subs r2, r2, r3
	add r2, r2, #127
	cmp r4, r5
	itt lo
	lsllo r4, r4, #1
	sublo r2, r2, #1

	/* Four digits into r3; r4 is the remainder */
	lsls r4, r4, #7
	udiv r3, r4, r5
	mls r4, r3, r5, r4
	lsls r4, r4, #8
	udiv r1, r4, r5
	mls r4, r1, r5, r4
	orr r3, r1, r3, lsl #8
	lsls r4, r4, #8
	udiv r1, r4, r5
	mls r4, r1, r5, r4
	orr r3, r1, r3, lsl #8
	lsls r4, r4, #8
	udiv r1, r4, r5
	mls r4, r1, r5, r4
	orr r3, r1, r3, lsl #8

	/* The product's layout: r5 the high word, r3 non-zero for the sticky remainder */
	mov r5, r3
	mov r3, r4
	/* The biased exponent e must lie in [1, 254] */
	sub r4, r2, #1
	cmp r4, #253
	bhi .Lrange

.Lpack:
	/* sign | (e - 1) 2^23 + r5 >> 8; round to nearest on bit 7, bits 6-0 of r5 and r3 being sticky */
	and r0, ip, #0x80000000
	subs r2, r2, #1
	add r0, r0, r2, lsl #23
	add r0, r0, r5, lsr #8
	lsls r4, r5, #24
	bpl 2f
	adds r0, r0, #1
	orrs r4, r3, r4, lsl #1
	it eq
	biceq r0, r0, #1
2:
	pop {r4, r5}
	bx lr

.Lrange:
	/* Above the range: infinity. Below it: shift right s = 1 - e (at most 31) with a sticky bit, onto the subnormal grid */
	cmp r2, #0
	bgt .Linfinity
	cmp r3, #0
	it ne
	orrne r5, r5, #1
	rsb r2, r2, #1
	cmp r2, #31
	it hi
	movhi r2, #31
	rsb r3, r2, #32
	lsl r3, r5, r3
	lsr r5, r5, r2
	movs r2, #1
	b .Lpack

.Linfinity:
	and r0, ip, #0x80000000
	orr r0, r0, #0x7f800000
	pop {r4, r5}
	bx lr
.Lzero:
	and r0, ip, #0x80000000
	pop {r4, r5}
	bx lr
.Lnan_a:
	orr r0, r0, #0x00400000
	pop {r4, r5}
	bx lr
.Lnan_b:
	orr r0, r1, #0x00400000
	pop {r4, r5}
	bx lr
.Ldefault_nan:
	movs r0, #0
	movt r0, #0x7fc0
	pop {r4, r5}
	bx lr

.Lspecial:
	lsl r4, r0, #1
	lsl r5, r1, #1
	cmp r4, #0xff000000
	bhi .Lnan_a
	cmp r5, #0xff000000
	bhi .Lnan_b
	cmp r4, #0xff000000
	beq .Linfinite_a
	cmp r5, #0xff000000
	beq .Lzero
	cmp r5, #0
	beq .Ldivide_by_zero
	cmp r4, #0
	beq .Lzero

	/* Normalise: the hidden bit at 23, a subnormal's exponent 1 - (CLZ - 8) */
	ubfx r4, r0, #0, #23
	cmp r2, #0
	bne 3f
	clz r2, r4
	subs r2, r2, #8
	lsl r4, r4, r2
	rsb r2, r2, #1
	b 4f
3:
	orr r4, r4, #0x00800000
4:
	ubfx r5, r1, #0, #23
	cmp r3, #0
	bne 5f
	clz r3, r5
	subs r3, r3, #8
	lsl r5, r5, r3
	rsb r3, r3, #1
	b .Lquotient
5:
	orr r5, r5, #0x00800000
	b .Lquotient

.Linfinite_a:
	/* Infinity over infinity is the default NaN; over anything finite, infinity */
	cmp r5, #0xff000000
	beq .Ldefault_nan
	b .Linfinity
.Ldivide_by_zero:
	/* 0/0 is the default NaN; anything else over zero is infinity */
	cmp r4, #0
	beq .Ldefault_nan
	b .Linfinity
	.size __aeabi_fdiv, . - __aeabi_fdiv

	.section .note.GNU-stack, "", %progbits
