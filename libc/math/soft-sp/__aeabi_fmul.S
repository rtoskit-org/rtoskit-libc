/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fmul.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a * b (IEEE 754-2019 5.4.1, RTABI 4.1.2) for Armv7-M, round to nearest,
 * ties to even, no exception flags (PLAN D44, D109,
 * docs/research/softfloat-thumb2.md).
 *
 * The significands with the hidden bit at bit 31 give the 64-bit product
 * r5:r3 from one UMULL; below 2^63 it shifts left once. r5 then holds the
 * 24-bit significand above the guard bit (bit 7); its bits 6-0 and r3 are
 * sticky. A result below the normal range is shifted right onto the
 * subnormal grid first, r3 folded into a sticky bit, so it is rounded
 * once. Subnormal operands are normalised with CLZ, their exponents then 0
 * or below. A NaN gives the first NaN, quiet; zero times infinity is the
 * default NaN (7.2); a zero or an infinity takes the signs' difference.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fmul,"ax",%progbits
	.global __aeabi_fmul
	.type __aeabi_fmul, %function
__aeabi_fmul:
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
	lsl r4, r0, #8
	orr r4, r4, #0x80000000
	lsl r5, r1, #8
	orr r5, r5, #0x80000000

.Lproduct:
	/* r5:r3 = r4 r5; e = ea + eb - 126, or one less after normalising a product below 2^63 */
	add r2, r2, r3
	umull r3, r5, r4, r5
	cmp r5, #0
	blt 1f
	adds r3, r3, r3
	adc r5, r5, r5
	subs r2, r2, #1
1:
	sub r2, r2, #126
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
	beq .Linfinite_b
	cmp r4, #0xff000000
	beq .Linfinite_a
	cmp r4, #0
	it ne
	cmpne r5, #0
	beq .Lzero

	/* Normalise: the hidden bit at 31, a subnormal's exponent 1 - CLZ */
	lsl r4, r0, #8
	cmp r2, #0
	bne 3f
	clz r2, r4
	lsl r4, r4, r2
	rsb r2, r2, #1
	b 4f
3:
	orr r4, r4, #0x80000000
4:
	lsl r5, r1, #8
	cmp r3, #0
	bne 5f
	clz r3, r5
	lsl r5, r5, r3
	rsb r3, r3, #1
	b .Lproduct
5:
	orr r5, r5, #0x80000000
	b .Lproduct

.Linfinite_a:
	/* Infinity times a finite b: zero gives the default NaN, otherwise infinity */
	cmp r5, #0
	beq .Ldefault_nan
	b .Linfinity
.Linfinite_b:
	cmp r4, #0
	beq .Ldefault_nan
	b .Linfinity
	.size __aeabi_fmul, . - __aeabi_fmul

	.section .note.GNU-stack, "", %progbits
