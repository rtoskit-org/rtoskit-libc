/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __libc_softfloat_compare.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The ordering of a (r0) and b (r1) in the flags, as CMP would set them
 * for two ordered integers (IEEE 754-2019 5.11; PLAN D109): Z set for
 * equal, C clear for less; unordered (a NaN on either side) is C set and
 * Z clear, which no condition the callers test reads as less or equal.
 * The zeros are equal. Otherwise two non-negatives compare as their bits,
 * two negatives as their bits reversed, and opposite signs by the sign
 * alone, the negative operand's bits being the larger. Only ip and the
 * flags change, so the callers keep r0-r3 (RTABI 4.1.2 requires it of
 * the __aeabi_cfcmp forms).
 */
	.syntax unified
	.thumb
	.section .text.__libc_softfloat_compare,"ax",%progbits
	.global __libc_softfloat_compare
	.hidden __libc_softfloat_compare
	.type __libc_softfloat_compare, %function
__libc_softfloat_compare:
	/* A NaN, the bits without the sign above infinity's, on either side is unordered */
	lsls ip, r0, #1
	cmp ip, #0xff000000
	bhi 3f
	lsls ip, r1, #1
	cmp ip, #0xff000000
	bhi 3f

	/* Two zeros are equal */
	orr ip, r0, r1
	lsls ip, ip, #1
	beq 4f

	/* Two non-negatives order as their bits */
	teq r0, r1
	bmi 1f
	cmp r0, #0
	blt 1f
	cmp r0, r1
	bx lr
1:
	/*
	 * Two negatives order as their bits reversed; opposite signs too, since
	 * the negative operand has the larger bits
	 */
	cmp r1, r0
	bx lr
3:
	/* Unordered: C set, Z clear */
	movs ip, #1
	cmp ip, #0
	bx lr
4:
	/* Equal: Z and C set */
	cmp r0, r0
	bx lr
	.size __libc_softfloat_compare, . - __libc_softfloat_compare

	.section .note.GNU-stack, "", %progbits
