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
 * r1:ip from one UMULL; below 2^63 it shifts left once. r1 then holds the
 * 24-bit significand above the guard bit (bit 7); its bits 6-0 and ip are
 * sticky. Packing adds the significand, whose hidden bit restores the
 * exponent, to the sign and the exponent less one, with the guard bit as
 * the carry in; a tie clears the last bit for the even neighbour. A result
 * below the normal range is shifted right onto the subnormal grid first,
 * ip folded into a sticky bit, so it is rounded once. Subnormal operands
 * are normalised with CLZ, their exponents then 0 or below. A NaN gives
 * the first NaN, quiet; zero times infinity is the default NaN (7.2); a
 * zero or an infinity takes the signs' difference.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fmul,"ax",%progbits
	.global __aeabi_fmul
	.type __aeabi_fmul, %function
__aeabi_fmul:
	/* An exponent field of 0 or 255 on either side: zero, subnormal, infinity or NaN */
	ubfx r2, r0, #23, #8
	ubfx r3, r1, #23, #8
	sub ip, r2, #1
	cmp ip, #254
	bhs .Lspecial
	sub ip, r3, #1
	cmp ip, #254
	bhs .Lspecial

	/* r2 = ea + eb, r3 the sign, the significands with the hidden bit at 31 */
	add r2, r2, r3
	eor r3, r0, r1
	lsl r0, r0, #8
	orr r0, r0, #0x80000000
	lsl r1, r1, #8
	orr r1, r1, #0x80000000

.Lproduct:
	/* r1:ip = r0 r1; e - 1 = ea + eb - 127, or one less after normalising a product below 2^63 */
	umull ip, r1, r0, r1
	cmp r1, #0
	blt 1f
	adds ip, ip, ip
	adc r1, r1, r1
	subs r2, r2, #1
1:
	sub r2, r2, #127

	/* e - 1 must lie in [0, 253] */
	cmp r2, #253
	bhi .Lrange

.Lpack:
	/* sign | (e - 1) 2^23 + r1 >> 8, the guard bit as carry in; a tie (r1's low byte 0x80 and ip zero) clears the last bit */
	and r0, r3, #0x80000000
	add r0, r0, r2, lsl #23
	and r2, r1, #0xff
	lsrs r1, r1, #8
	adc r0, r0, r1
	eor r2, r2, #0x80
	orrs r2, r2, ip
	it eq
	biceq r0, r0, #1
	bx lr

.Lrange:
	/* Above the range: infinity. Below it: shift right s = -(e - 1) (at most 31) with a sticky bit, onto the subnormal grid */
	cmp r2, #0
	bgt .Linfinity
	cmp ip, #0
	it ne
	orrne r1, r1, #1
	rsb r2, r2, #0
	cmp r2, #31
	it hi
	movhi r2, #31
	rsb ip, r2, #32
	lsl ip, r1, ip
	lsr r1, r1, r2
	movs r2, #0
	b .Lpack

.Linfinity:
	and r0, r3, #0x80000000
	orr r0, r0, #0x7f800000
	bx lr

.Lspecial:
	/* A zero b first (x * 0 is the common special case); then an infinite or NaN operand; then a zero a */
	lsls ip, r1, #1
	beq .Lzero_b
	cmp ip, #0xff000000
	bhs .Lspecial_b
	lsls ip, r0, #1
	cmp ip, #0xff000000
	bhs .Lspecial_a
	lsls ip, r0, #1
	it ne
	lslsne ip, r1, #1
	beq .Lsigned_zero

	/* Normalise: the hidden bit at 31, a subnormal's exponent 1 - CLZ */
	eor ip, r0, r1
	lsl r0, r0, #8
	cmp r2, #0
	bne 2f
	clz r2, r0
	lsl r0, r0, r2
	rsb r2, r2, #1
	b 3f
2:
	orr r0, r0, #0x80000000
3:
	lsl r1, r1, #8
	cmp r3, #0
	bne 4f
	clz r3, r1
	lsl r1, r1, r3
	rsb r3, r3, #1
	b 5f
4:
	orr r1, r1, #0x80000000
5:
	add r2, r2, r3
	mov r3, ip
	b .Lproduct

.Lspecial_b:
	/* b a NaN: the first NaN, quiet. b infinite: a NaN first, then zero times infinity is the default NaN */
	bhi 6f
	lsls ip, r0, #1
	cmp ip, #0xff000000
	bhi .Lnan_a
	cmp ip, #0
	beq .Ldefault_nan
	b .Lsigned_infinity
6:
	lsls ip, r0, #1
	cmp ip, #0xff000000
	bhi .Lnan_a
	b .Lnan_b
.Lspecial_a:
	/* a a NaN: quiet. a infinite (b finite): times zero the default NaN, otherwise infinity */
	bhi .Lnan_a
	lsls ip, r1, #1
	beq .Ldefault_nan
	b .Lsigned_infinity
.Lzero_b:
	/* a NaN: quiet; infinity times zero: the default NaN; otherwise a signed zero */
	lsls ip, r0, #1
	cmp ip, #0xff000000
	bhi .Lnan_a
	beq .Ldefault_nan
.Lsigned_zero:
	eor r0, r0, r1
	and r0, r0, #0x80000000
	bx lr
.Lsigned_infinity:
	eor r0, r0, r1
	and r0, r0, #0x80000000
	orr r0, r0, #0x7f800000
	bx lr
.Lnan_a:
	orr r0, r0, #0x00400000
	bx lr
.Lnan_b:
	orr r0, r1, #0x00400000
	bx lr
.Ldefault_nan:
	movs r0, #0
	movt r0, #0x7fc0
	bx lr
	.size __aeabi_fmul, . - __aeabi_fmul

	.section .note.GNU-stack, "", %progbits
