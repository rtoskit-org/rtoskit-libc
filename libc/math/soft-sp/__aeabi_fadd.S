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
 * a + b (IEEE 754-2019 5.4.1, RTABI 4.1.2) for Armv7-M, round to nearest,
 * ties to even, no exception flags (PLAN D44, D109,
 * docs/research/softfloat-thumb2.md). __aeabi_fsub and __aeabi_frsub flip
 * a sign and come here.
 *
 * The operands are ordered by magnitude, so a decides the sign, and their
 * significands unpacked with the hidden bit at bit 30; a subnormal takes
 * exponent 1 and no hidden bit, so both share one grid. b is shifted right
 * by the exponent difference (at most 31, beyond which it is only sticky),
 * the bits shifted out setting bit 0. That bit keeps every sum and
 * difference odd whenever it stands for lost bits, so no rounding boundary
 * can fall between the computed and the exact result. A carry into bit 31
 * shifts right once, keeping its sticky bit; a difference is normalised by
 * CLZ, but never below exponent 1, so a subnormal result is exact.
 *
 * Packing adds (exponent - 1) 2^23 to the significand's bits 30-7, whose
 * hidden bit restores the exponent; bit 6 is the guard bit and bits 5-0
 * sticky. A rounding carry moves into the exponent, and from the largest
 * finite value it gives infinity's encoding. A NaN gives that NaN, quiet
 * (of two, the larger magnitude); infinity minus infinity is the default
 * NaN (7.2); x - x is +0, and -0 + -0 is -0 (6.3).
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
	eor r0, r0, r1
	eor r1, r1, r0
	eor r0, r0, r1
	eor r2, r2, r3
	eor r3, r3, r2
	eor r2, r2, r3
1:
	/* a infinite or NaN; b a zero */
	cmp r2, #0xff000000
	bhs .Lspecial
	cmp r3, #0
	beq .Lzero_b
	push {r4, r5}

	/* Exponent fields, and the significands with the hidden bit at 30 (none, exponent 1, for a subnormal) */
	lsrs r2, r2, #24
	lsrs r3, r3, #24
	lsl r4, r0, #8
	lsl r5, r1, #8
	cmp r2, #0
	ite ne
	orrne r4, r4, #0x80000000
	moveq r2, #1
	cmp r3, #0
	ite ne
	orrne r5, r5, #0x80000000
	moveq r3, #1
	lsrs r4, r4, #1
	lsrs r5, r5, #1

	/* b shifted right by the difference d, at most 31, its lost bits setting bit 0 */
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
	bmi .Lsubtract

	/* Same signs: a carry into bit 31 shifts right once, the exponent up one */
	adds r4, r4, r5
	bpl .Lround
	and ip, r4, #1
	orr r4, ip, r4, lsr #1
	adds r2, r2, #1
	cmp r2, #255
	bhs .Linfinity
	b .Lround

.Lsubtract:
	/* Opposite signs: exact cancellation is +0; otherwise normalise, the exponent staying at least 1 */
	subs r4, r4, r5
	beq .Lplus_zero
	clz r3, r4
	subs r3, r3, #1
	cmp r3, r2
	it hs
	subhs r3, r2, #1
	lsl r4, r4, r3
	sub r2, r2, r3

.Lround:
	/* a's sign | (exponent - 1) 2^23 + bits 30-7; round to nearest, a tie to the even neighbour */
	and r0, r0, #0x80000000
	subs r2, r2, #1
	add r0, r0, r2, lsl #23
	add r0, r0, r4, lsr #7
	lsls ip, r4, #25
	bpl 2f
	adds r0, r0, #1
	lsls ip, ip, #1
	it eq
	biceq r0, r0, #1
2:
	pop {r4, r5}
	bx lr

.Linfinity:
	and r0, r0, #0x80000000
	orr r0, r0, #0x7f800000
	pop {r4, r5}
	bx lr

.Lplus_zero:
	movs r0, #0
	pop {r4, r5}
	bx lr

.Lzero_b:
	/* x + 0 is x; of two zeros, the sum is -0 only when both are */
	cmp r2, #0
	it eq
	andeq r0, r0, r1
	bx lr

.Lspecial:
	/* a NaN: quietened; a infinite: infinity minus infinity is the default NaN, otherwise a */
	bhi .Lnan
	cmp r3, r2
	bne 3f
	teq r0, r1
	bpl 3f
	movs r0, #0
	movt r0, #0x7fc0
3:
	bx lr
.Lnan:
	orr r0, r0, #0x00400000
	bx lr
	.size __aeabi_fadd, . - __aeabi_fadd

	.section .note.GNU-stack, "", %progbits
