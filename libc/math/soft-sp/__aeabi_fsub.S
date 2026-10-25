/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_fsub.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * a - b (IEEE 754-2019 5.4.1, RTABI 4.1.2; PLAN D109): a + (-b), the
 * negation flipping the sign bit only, which changes no rounding.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_fsub,"ax",%progbits
	.global __aeabi_fsub
	.type __aeabi_fsub, %function
__aeabi_fsub:
	eor r1, r1, #0x80000000
	b __aeabi_fadd
	.size __aeabi_fsub, . - __aeabi_fsub

	.section .note.GNU-stack, "", %progbits
