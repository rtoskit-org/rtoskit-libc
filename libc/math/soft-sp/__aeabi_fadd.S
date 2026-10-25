/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fadd.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
/*
 * a + b (IEEE 754-2019 5.4.1, RTABI 4.1.2) for Armv7-M, round to nearest,
 * ties to even, no exception flags (PLAN D44, D109,
 * docs/research/softfloat-thumb2.md). __aeabi_fsub and __aeabi_frsub flip
 * a sign and come here.
 *
 * The operands are ordered by magnitude, so a decides the sign. Two normal
 * operands take the fast path: from an exponent difference d of 26, b is
 * below a quarter of a's ulp and the sum rounds to a; otherwise both
 * significands go to the top of a word (hidden bit 31, guard and sticky
 * room in bits 7-0), b is shifted right by d and its lost bits set bit 0.
 * That bit keeps every sum and difference odd whenever it stands for lost
 * bits, so no rounding boundary falls between the computed and the exact
 * result. A sum's carry out of bit 31 comes back through RRX, keeping the
 * sticky bit; a difference is normalised by CLZ, never below exponent 1,
 * so a subnormal result is exact.
 *
 * r2 carries a's sign and the exponent less one, so packing adds the
 * significand's bits 31-8, whose hidden bit restores the exponent, with
 * the guard bit (bit 7) as the carry in; a tie (bits 6-0 zero) clears the
 * last bit for the even neighbour. A rounding carry moves into the
 * exponent, and from the largest finite value it is infinity's encoding.
 * A subnormal b goes the general way, on a grid where a subnormal has
 * exponent 1 and no hidden bit. A NaN gives that NaN, quiet (of two, the
 * larger magnitude); infinity minus infinity is the default NaN (7.2);
 * x - x is +0, and -0 + -0 is -0 (6.3).
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fadd,"ax",%progbits
	.global __aeabi_fadd
	.type __aeabi_fadd, %function
__aeabi_fadd:
	/* |a| >= |b| from here on */
	lsls r2, r0, #1
	lsls r3, r1, #1
	cmp r2, r3
	bhs 1f
	mov ip, r0
	mov r0, r1
	mov r1, ip
	mov ip, r2
	mov r2, r3
	mov r3, ip
1:
	/* b zero; a infinite or NaN; b subnormal (exponent field 0) */
	cmp r3, #0
	beq .Lzero_b
	cmp r2, #0xff000000
	bhs .Lspecial
	lsrs r2, r2, #24
	lsrs r3, r3, #24
	beq .Lgeneral

	/* From d = 26, a + b rounds to a */
	subs r3, r2, r3
	cmp r3, #26
	bhs .Lreturn

	/* r2 = a's sign | (exponent - 1) 2^23 */
	lsrs r2, r0, #23
	subs r2, r2, #1
	lsls r2, r2, #23
	teq r0, r1
	bmi .Lsubtract

	/* Same signs: hidden bits at 31, b shifted right by d with a sticky bit 0, then the sum */
	lsl r0, r0, #8
	orr r0, r0, #0x80000000
	lsl r1, r1, #8
	orr r1, r1, #0x80000000
	rsb ip, r3, #32
	lsls ip, r1, ip
	lsr r1, r1, r3
	it ne
	orrne r1, r1, #1
	adds r0, r0, r1
	bcs .Lcarry

.Lpack:
	/* r2 + bits 31-8, the guard bit as carry in; a tie clears the last bit */
	and ip, r0, #0xff
	lsrs r0, r0, #8
	adc r0, r2, r0
	cmp ip, #0x80
	it eq
	biceq r0, r0, #1
	bx lr

.Lcarry:
	/* The sum reached 2^32: RRX shifts the carry back in, bit 0 stays sticky, the exponent goes up one */
	and ip, r0, #1
	rrx r0, r0
	orr r0, r0, ip
	add r2, r2, #0x00800000
	lsls ip, r2, #1
	cmp ip, #0xfe000000
	blo .Lpack
	and r0, r2, #0x80000000
	orr r0, r0, #0x7f800000
	bx lr

.Lsubtract:
	/* Opposite signs: the difference, then CLZ normalisation, the exponent staying at least 1 */
	lsl r0, r0, #8
	orr r0, r0, #0x80000000
	lsl r1, r1, #8
	orr r1, r1, #0x80000000
	rsb ip, r3, #32
	lsls ip, r1, ip
	lsr r1, r1, r3
	it ne
	orrne r1, r1, #1
	subs r0, r0, r1
	beq .Lplus_zero
	clz r3, r0
	ubfx ip, r2, #23, #8
	cmp r3, ip
	it hi
	movhi r3, ip
	lsl r0, r0, r3
	sub r2, r2, r3, lsl #23
	b .Lpack

.Lplus_zero:
	movs r0, #0
.Lreturn:
	bx lr

.Lzero_b:
	/* x + 0 is x, a NaN quietened; of two zeros, the sum is -0 only when both are */
	cmp r2, #0xff000000
	bhi .Lnan
	cmp r2, #0
	it eq
	andeq r0, r0, r1
	bx lr

.Lgeneral:
	/* A subnormal b (and maybe a): significands with the hidden bit at 30, exponent 1 and no hidden bit for a subnormal */
	push {r4, r5}
	lsl r4, r0, #8
	lsl r5, r1, #8
	cmp r2, #0
	ite ne
	orrne r4, r4, #0x80000000
	moveq r2, #1
	movs r3, #1
	lsrs r4, r4, #1
	lsrs r5, r5, #1

	/* b shifted right by the difference d (below 254, so at most 31 matters), its lost bits setting bit 0 */
	subs r3, r2, r3
	cmp r3, #31
	it hi
	movhi r3, #31
	rsb ip, r3, #32
	lsl ip, r5, ip
	lsr r5, r5, r3
	cmp ip, #0
	it ne
	orrne r5, r5, #1

	teq r0, r1
	bmi 2f

	/* Same signs: a carry into bit 31 shifts right once, keeping its sticky bit, the exponent up one */
	adds r4, r4, r5
	bpl 3f
	and ip, r4, #1
	orr r4, ip, r4, lsr #1
	adds r2, r2, #1
	b 3f
2:
	/* Opposite signs: exact cancellation is +0; otherwise normalise, the exponent staying at least 1 */
	subs r4, r4, r5
	beq 5f
	clz r3, r4
	subs r3, r3, #1
	cmp r3, r2
	it hs
	subhs r3, r2, #1
	lsl r4, r4, r3
	sub r2, r2, r3
3:
	/* a's sign | (exponent - 1) 2^23 + bits 30-7; round to nearest on bit 6, a tie to the even neighbour */
	and r0, r0, #0x80000000
	subs r2, r2, #1
	add r0, r0, r2, lsl #23
	add r0, r0, r4, lsr #7
	lsls ip, r4, #25
	bpl 4f
	adds r0, r0, #1
	lsls ip, ip, #1
	it eq
	biceq r0, r0, #1
4:
	pop {r4, r5}
	bx lr
5:
	movs r0, #0
	pop {r4, r5}
	bx lr

.Lspecial:
	/* a NaN: quietened; a infinite: infinity minus infinity is the default NaN, otherwise a */
	bhi .Lnan
	cmp r3, r2
	bne 6f
	teq r0, r1
	bpl 6f
	movs r0, #0
	movt r0, #0x7fc0
6:
	bx lr
.Lnan:
	orr r0, r0, #0x00400000
	bx lr
	.size __aeabi_fadd, . - __aeabi_fadd

	.section .note.GNU-stack, "", %progbits
