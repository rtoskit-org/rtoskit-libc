/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_cfcmpeq.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The ordering of a and b in the flags (RTABI 4.1.2; PLAN D109), as
 * __aeabi_cfcmple: the two differ only in signalling invalid for a quiet
 * NaN, and these cores raise no flags (D44). r0-r3 kept.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_cfcmpeq,"ax",%progbits
	.global __aeabi_cfcmpeq
	.type __aeabi_cfcmpeq, %function
__aeabi_cfcmpeq:
	b __libc_softfloat_compare
	.size __aeabi_cfcmpeq, . - __aeabi_cfcmpeq

	.section .note.GNU-stack, "", %progbits
