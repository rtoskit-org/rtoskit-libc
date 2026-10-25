/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_frsub.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * b - a (RTABI 4.1.2, the reverse subtraction; PLAN D109): (-a) + b.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_frsub,"ax",%progbits
	.global __aeabi_frsub
	.type __aeabi_frsub, %function
__aeabi_frsub:
	eor r0, r0, #0x80000000
	b __aeabi_fadd
	.size __aeabi_frsub, . - __aeabi_frsub

	.section .note.GNU-stack, "", %progbits
